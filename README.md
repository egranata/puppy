# The Puppy Kernel

Welcome to Puppy. Our current build status is: [![](https://travis-ci.org/egranata/puppy.svg?branch=master)](https://travis-ci.org/egranata/puppy)

The Puppy project aims to build an operating system for x86 computers.

More specifically, project goals include providing:

- preemptive multitasking of userspace processes (but not a preemptive kernel); ✅
- support for memory protection; ✅
- system call interface to userspace; ✅
- booting on emulators **AND** real hardware; ✅
- FAT filesystem on hard disks; ✅
- program launcher (aka, shell);
- a C/C++ standard library.

Development of a GUI or porting to architectures other than 32-bit Intel (including but not limited to x86_64 or any ARM flavor) are explicit non-goals. It is also not a goal to develop a custom bootloader, Puppy just uses GRUB.

Support for network connectivity, USB, or other interesting hardware would be nice to have.

# Hardware requirements

Most Puppy development and testing happens on QEMU, but the OS can also boot Bochs and - assuming a few hardware requirements are met - it will work on a real computer. If your hardware has a PS/2 keyboard and an IDE hard disk (or proper emulation of both...), then Puppy should boot. Feel free to test it out! I haven't tried other virtualizers, e.g. VirtualBox, VMWare, ..., but testing on those is definitely welcome.

## Getting Started

Puppy is a C++ codebase. To build the OS into a bootable image, one needs:
- an installation of Linux (or a Docker container thereof);
- the NASM assembler;
- a GCC cross compiler, as described at [http://wiki.osdev.org/GCC_Cross-Compiler];
- Python 3;
- GRUB 2;
- Xorriso.

A reasonable approximation of the dependencies required and how to get them in place is described at `build/deps.sh`.

Assuming those are in place, to compile the project, just type

```
$ ./build.py
```

in a shell. This will churn for a while (should be under a minute) and produce a bootable HD image `out/os.img`.

The HD image includes the kernel, as well as a fairly minimal userspace + suite of tests. It can be used to boot an emulator, or bit-blasted to a real hard disk.

A sample configuration for Bochs and QEMU launcher script can be found in the `build` directory.

Please be aware that **this is not an officially supported Google product**.
