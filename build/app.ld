ENTRY(___start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 0x400000;

	__module_start = .;

    .text : AT(ADDR(.text)) {
		*(.text)
	}

	.rodata ALIGN (0x1000) : AT(ADDR(.rodata)) {
		*(.rodata*)
	}

	.data ALIGN (0x1000) : AT(ADDR(.data)) {
		*(.data)
	}

	.bss ALIGN (0x1000) : AT(ADDR(.bss)) {
        *(COMMON)
        *(.bss)
    }
	
	__ctors_start = .;
	.ctors : AT(ADDR(.ctors)) {
		*(.ctors)
	}
	__ctors_end = .;

	__dtors_start = .;
	.dtors : AT(ADDR(.dtors)) {
		*(.dtors)
	}
	__dtors_end = .;

	__module_end = .;
}
