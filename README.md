# The Puppy Kernel
[![](https://travis-ci.org/egranata/puppy.svg?branch=master)](https://travis-ci.org/egranata/puppy)

Welcome to Puppy.

Puppy is an operating system for IBM-compatible computers with 32-bit Intel CPUs (you may have heard of this kind of hardware under the name *personal computer*).

More specifically, Puppy aims to provide:

- preemptive multitasking (but not a preemptive kernel); ✅
- memory protection; ✅
- system calls for userspace; ✅
- FAT32 filesystems; ✅
- support for actual physical x86 hardware; ✅
- a C/C++ standard library for userspace programs;
- ACPI support;
- a functional userspace (custom built or ported as needed).

Some additional goals and features are covered by https://github.com/egranata/puppy/issues. Even more things (e.g. support for USB, networking, ...) are definitely interesting and may become important as the core goals are fulfilled.

Porting to non-x86 architectures or development of a GUI are explicit non goals. It is also not a goal to develop custom bootloaders: GRUB is plenty enough!

# Hardware requirements

Puppy is actively tested on QEMU - and occasionally Bochs.
![](docs/qemu.png)

![](docs/bochs.png)

Assuming a few requirements are met, Puppy should boot and work on an actual PC:
- PS/2 keyboard;
- A video card and screen with VBE support and capable of 800x600 or better;
- APIC timer;
- Intel CPU from - at least - the early-2000s (it might and should work on earlier HW, but it is untested);
- at least 1GB of RAM (again, doesn't need nearly as much, but is untested).

If your system has a physical serial port, you should be able to collect kernel logs which might help in diagnosing boot-time issue. I personally use `picocom` for this purpose, but other emulators should work if they're capable of 8N1 115200 operation.

Puppy is untested on other emulators/VMs, such as VirtualBox and/or VMWare. Testing is welcome!

# Software requirements

To compile a Puppy image, you'll want to use Linux (native or in a VM/container - I do most of my development in Docker). Dependencies for compilation are aptly described by `build/deps.sh` (the script used to setup a CI instance).

# Getting started

Assuming you have the proper bits and pieces in place, to compile the project, just type

```
$ ./build.py
```

in a shell. This will churn for a while (should be under a minute) and produce a bootable HD image `out/os.img`.

The HD image includes the kernel, as well as a fairly minimal userspace + suite of tests. It can be used to boot an emulator, or bit-blasted to a real hard disk.

A sample configuration for Bochs and QEMU launcher script can be found in the `build` directory.

Please be aware that **this is not an officially supported Google product**.

# Contributing

Please see `CONTRIBUTING.md` and follow the instructions therein.
