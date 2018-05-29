# The Puppy Kernel

Puppy is a simple-yet-functional x86 operating system kernel.

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

GRUB is assumed to be the bootloader of choice for Puppy. Writing a bootloader is **not** a goal of Puppy.

Puppy is a C++ codebase, compilable using NASM and a GCC cross compiler set up as per [http://wiki.osdev.org/GCC_Cross-Compiler].
To compile the project, just type

```
$ ./build.py
```

in a shell. This will compile the system and produce a bootable ISO image at `out/os.iso`

Please be aware that **this is not an officially supported Google product**.
