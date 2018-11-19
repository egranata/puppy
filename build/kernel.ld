ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 0xC0100000;

	__kernel_start = .;

    .grub : AT(ADDR(.grub) - 0xC0000000) {
		KEEP(*(.grub))
		*(.grub)
	}

	.text : AT(ADDR(.text) - 0xC0000000) {
		*(.text*)
	}

	.ctors : AT(ADDR(.ctors) - 0xC0000000) {
		__ctors_start = .;
		KEEP(*(.ctors))
		*(.ctors)
		__ctors_end = .;
	}

	.rodata ALIGN (0x1000) : AT(ADDR(.rodata)-0xC0000000) {
		*(.rodata*)
	}

	.data ALIGN (0x1000) : AT(ADDR(.data)-0xC0000000) {
		*(.data)            /* all data sections from all files */
	}

	.bss ALIGN (0x1000) : AT(ADDR(.bss)-0xC0000000) {
        *(COMMON)
        *(.bss*)
    }
	
	.acpi_ddriv ALIGN (0x1000) : AT(ADDR(.acpi_ddriv)-0xC0000000) {
		__acpi_ddriv_start = .;
		KEEP(*(.acpi_ddriv))
		*(.acpi_ddriv)
		__acpi_ddriv_end = .;
	}

	.pci_ddriv ALIGN (0x1000) : AT(ADDR(.pci_ddriv)-0xC0000000) {
		__pci_ddriv_start = .;
		KEEP(*(.pci_ddriv))
		*(.pci_ddriv)
		__pci_ddriv_end = .;
	}

	__kernel_end = .;
}
