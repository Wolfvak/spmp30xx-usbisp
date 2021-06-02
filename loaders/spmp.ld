OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
	RAM (RWX) : ORIGIN = 0x24000000, LENGTH = 0x1000
	/*
	 * in theory the size can be 16MiB
	 * but I guess it's best to keep it small
	 * this is only meant for really simple payloads
	 */
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
