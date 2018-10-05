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
- a C/C++ standard library for userspace programs; ✅
- ACPI support;
- a functional userspace (custom built or ported as needed).

Some additional goals and features are covered by https://github.com/egranata/puppy/issues. Features such as USB and/or networking are definitely interesting and may become important as the core goals are fulfilled.

On the other hands, there are some things that are explicit **non-goals** and as such will not be worked on:

- porting to anything other than x86;
- a GUI (either porting or writing a new one);
- developing a custom bootloader.

# Hardware requirements

Puppy is actively tested on QEMU - and occasionally Bochs.
![](docs/qemu.png)

![](docs/bochs.png)

Assuming a few requirements are met, Puppy should boot and work on an actual PC:
- PS/2 keyboard;
- A video card and screen with VBE support and capable of 800x600 or better;
- APIC timer;
- Intel CPU from - at least - the early-2000s (*);
- at least 1GB of RAM (should not need nearly as much, but is untested).

(*) Testing with Bochs suggests that Puppy can boot on something as old as a Pentium Pro, but this is untested on real hardware.
Alas, booting on anything older is currently not possible (see https://github.com/egranata/puppy/issues/63).

If your system has a physical serial port, you should be able to collect kernel logs which might help in diagnosing boot-time issue. I personally use `picocom` for this purpose, but other tools should work if they're capable of 8N1 115200 operation.

Testing on other virtualizers and/or real hardware is definitely most welcome.

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

# Contributing

Contributions to the project are very welcome.

The issues list (https://github.com/egranata/puppy/issues) is actively maintained with new units of work, and issues that would benefit from help and/or are great starting points are marked as such.

If you would like to try porting userspace software to Puppy, or add drivers for new hardware, that is also welcome - any and all issues you encounter doing this are definitely worth reporting.

In order to expedite your contribution all the way to acceptance, please see `CONTRIBUTING.md` and follow the instructions therein.

Please be aware that **this is not an officially supported Google product**.
