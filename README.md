# Compiling

The gist:

```bash-session
$ make -C kernel-source/ M=$PWD
```

For us:

```bash-session
$ make -C ~/Git/tinyware/build/sources/linux-6.3-rc3 CROSS_COMPILE=arm-linux-gnueabihf ARCH=arm O=~/Git/tinyware/build/boneblack/linux/build M=/home/edtwardy/Git/nunchuk-driver
```

# Formatting

```bash-session
$ clang-format -i -style=file:/home/edtwardy/Git/tinyware/build/sources/linux-6.3-rc3/.clang-format nunchuk.c
```
