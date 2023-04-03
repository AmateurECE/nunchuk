# Compiling

From the current directory:

```bash-session
$ make -C kernel-source/ M=$PWD
```

Don't forget to set `CROSS_COMPILE` AND `ARCH` for cross-compilation, and `O`
for out-of-tree builds!

# Formatting

```bash-session
$ clang-format -i -style=file:kernel-source/.clang-format nunchuk.c
```
