// SPDX-License-Identifier: GPL-2.0+
// Author: Ethan D. Twardy <ethan.twardy@plexus.com>

// Great driver development guide here: https://www.kernel.org/doc/html/latest/input/input-programming.html

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>

struct nunchuk_device {
	struct input_dev *input;
};

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

	nunchuk->input = devm_input_allocate_device(&client->dev);
	if (NULL == nunchuk->input) {
		return -ENOMEM;
	}

	result = input_register_device(nunchuk->input);
	i2c_set_clientdata(client, nunchuk);
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
