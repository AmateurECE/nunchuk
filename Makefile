# Read more here: https://docs.kernel.org/kbuild/modules.html
# For more complicated modules, we can split these up into two files: Kbuild and Makefile

ifneq ($(KERNELRELEASE),)
# We were invoked from Kbuild
obj-m := nunchuk.o

else
# We were invoked from the command line
all:
		$(MAKE) -C $(KBUILD_SRC) M=$$PWD

endif
