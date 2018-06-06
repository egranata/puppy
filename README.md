# The Puppy Kernel

Welcome to Puppy. This project aims to produce a simple-yet-functional operating system for x86 computers.

Explicit goals of the project include providing:

- preemptive multitasking of userspace processes (but not a preemptive kernel);
- support for memory protection;
- FAT filesystem on hard disks;
- system call interface to userspace;
- program launcher (aka, shell);
- a C/C++ standard library.

Developing a graphical UI, or providing network connectivity, or ports to any architecture other than x86 (including x86_64) are non-goals of the project.

Puppy currently runs on real hardware (assuming it has a *real* PS/2 keyboard), detects IDE hard disks, and can mount FAT filesystems off of them.
It also supports booting QEMU and Bochs.

GRUB is assumed to be the bootloader of choice for Puppy. Writing a bootloader is not a goal of Puppy.

## Getting Started

Puppy is a C++ codebase. To build the OS into a bootable image, one needs:
- an installation of Linux (or a Docker container thereof);
- the NASM assembler;
- a GCC cross compiler, as described at [http://wiki.osdev.org/GCC_Cross-Compiler];
- Python 3;
- GRUB 2;
- Xorriso.

Once the dependencies are satisfied, to compile the project, just type

```
$ ./build.py
```

in a shell. This will compile the system and produce a:
- a bootable ISO image `out/os.iso`;
- a bootable HD image `out/os.img`.

QEMU and Bochs can be used to test the deployed image. A sample bochs configuration and QEMU launcher script are provided in the `build` directory.

Please be aware that **this is not an officially supported Google product**.
