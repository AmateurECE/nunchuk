// SPDX-License-Identifier: GPL-2.0+
// Author: Ethan D. Twardy <ethan.twardy@plexus.com>

// Great driver development guide here: https://www.kernel.org/doc/html/latest/input/input-programming.html

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>

struct nunchuk_device {
	struct i2c_client *client;
	struct input_dev *input;
};

struct nunchuk_registers {
	bool z;
	bool c;
};

static int nunchuk_read_registers(struct i2c_client *client,
				  struct nunchuk_registers *registers)
{
	int result = 0;
	const char read_command[] = { 0x00 };
	char reg_value[6] = { 0 };

	// For the Nunchuk, each read triggers the next read.
	result = i2c_master_send(client, read_command, sizeof(read_command));
	if (sizeof(read_command) != result) {
		dev_err(&client->dev,
			"Failed to send read command to nunchuk!");
		return result;
	}

	usleep_range(1900, 2100);

	result = i2c_master_recv(client, reg_value, sizeof(reg_value));
	if (sizeof(reg_value) != result) {
		dev_err(&client->dev, "Failed to read register from nunchuk!");
		return result;
	}

	registers->z = !(reg_value[5] & BIT(0));
	registers->c = !(reg_value[5] & BIT(1));
	return 0;
}

static void nunchuk_poll(struct input_dev *input)
{
	int result = 0;
	struct nunchuk_registers registers = { 0 };
	struct nunchuk_device *nunchuk = input_get_drvdata(input);

	result = nunchuk_read_registers(nunchuk->client, &registers);
	if (0 != result) {
		return;
	}

	input_report_key(input, BTN_C, registers.c);
	input_report_key(input, BTN_Z, registers.z);
	input_sync(input);
}

static int nunchuk_init(struct nunchuk_device *nunchuk)
{
	// Bunch of magic here. Why is it magic? Because the only public documentation on the Wii Nunchuk I2C interface was created by reverse engineers.
	const char first_command[] = { 0xf0, 0x55 };
	const char second_command[] = { 0xfb, 0x00 };
	int result = 0;
	struct i2c_client *client = nunchuk->client;
	struct nunchuk_registers registers = { 0 };

	result = i2c_master_send(client, first_command, sizeof(first_command));
	if (sizeof(first_command) != result) {
		dev_err(&client->dev,
			"Failed to send first command to nunchuk!");
		return result;
	}

	// See https://docs.kernel.org/timers/timers-howto.html
	usleep_range(900, 1100);

	result =
		i2c_master_send(client, second_command, sizeof(second_command));
	if (sizeof(second_command) != result) {
		dev_err(&client->dev,
			"Failed to send second command to nunchuk!");
		return result;
	}

	result = nunchuk_read_registers(nunchuk->client, &registers);
	if (0 != result) {
		return result;
	}

	return 0;
}

// Probe--the i2c core has determined that our driver matches `client`.
static int nunchuk_probe(struct i2c_client *client)
{
	int result = 0;

	// https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html
	struct nunchuk_device *nunchuk = devm_kzalloc(
		&client->dev, sizeof(struct nunchuk_device), GFP_KERNEL);
	if (NULL == nunchuk) {
		return -ENOMEM;
	}

	nunchuk->client = client;
	nunchuk->input = devm_input_allocate_device(&client->dev);
	if (NULL == nunchuk->input) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, nunchuk);
	input_set_drvdata(nunchuk->input, nunchuk);

	// Set up some basic information about the input device
	nunchuk->input->name = "Wii Nunchuk";
	nunchuk->input->id.bustype = BUS_I2C;

	// Tell the input subsystem which events we generate
	set_bit(EV_KEY, nunchuk->input->evbit);
	set_bit(BTN_C, nunchuk->input->keybit);
	set_bit(BTN_Z, nunchuk->input->keybit);

	input_setup_polling(nunchuk->input, nunchuk_poll);
	input_set_poll_interval(nunchuk->input, 20);
	dev_info(&client->dev, "Nunchuk polling at %d msec",
		 input_get_poll_interval(nunchuk->input));

	// Initialize the Nunchuk
	result = nunchuk_init(nunchuk);
	if (0 != result) {
		return result;
	}

	// Must be ready to start polling by this point!
	result = input_register_device(nunchuk->input);
	return result;
}

static void nunchuk_remove(struct i2c_client *client)
{
	struct nunchuk_device *nunchuk = i2c_get_clientdata(client);
	input_unregister_device(nunchuk->input);
}

static const struct i2c_device_id nunchuk_ids[] = {
	{ "nunchuk-white",
	  0 }, // Second parameter is a pointer to some data. This can be used to inject device-specific configuration into probe.
	{}, // Sentinel
};
MODULE_DEVICE_TABLE(i2c, nunchuk_ids);

#ifdef CONFIG_OF
static const struct of_device_id nunchuk_of_ids[] = {
	{
		.compatible = "nintendo,nunchuk-white",
	},
	{},
};
MODULE_DEVICE_TABLE(of, nunchuk_of_ids);
#endif

static struct i2c_driver nunchuk_driver = {
    // Use `.probe_new` instead of `.probe`, because the latter has a second parameter which is almost never used.
    .probe_new = nunchuk_probe,
    .remove = nunchuk_remove,
    .id_table = nunchuk_ids,
    .driver = {
        .name = "nunchuk",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(nunchuk_of_ids),
    },
};

module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Ethan D. Twardy <ethan.twardy@plexus.com>");
MODULE_DESCRIPTION("Driver for the Wii Nunchuk Controller");
