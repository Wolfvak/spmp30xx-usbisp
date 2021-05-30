OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
	RAM (RWX) : ORIGIN = 0x24000000, LENGTH = 0x1000
}

SECTIONS
{
	.text : ALIGN(4) {
		*(.text*);
	} >RAM

	.data : ALIGN(4) {
		*(.data*);
		*(.rodata*);
	} >RAM

	.bss : ALIGN(4) {
		*(.bss*);
	} >RAM
}
