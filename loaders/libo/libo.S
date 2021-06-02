.cpu arm926ej-s
.fpu softvfp

.set DTB_ADDRESS, 0x24001000
.set KERNEL_ADDRESS, 0x24008000

.set ACTIVE_UART, 1
.set REG_UART_BASE, (0x10001800 + ACTIVE_UART * 32)

.set REG_UART_FIFO, 0x02
.set REG_UART_STAT, 0x0A
.set UART_STAT_TX_BUSY, 2

SCRATCH0 .req r0
SCRATCH1 .req r1
SCRATCH2 .req r2
SCRATCH3 .req r3
PARAM0 .req r4
PARAM1 .req r5
PARAM2 .req r6
UARTBASE .req r7

.arm

.align 2

.section .text
.global _start
.func _start
_start:
	msr cpsr, #0xD3 @ immediately switch to supervisor mode, IRQ/FIQs disabled

dcloop: @ test, clean and invalidate all dcache
	mrc p15, 0, r15, c7, c14, 3
	bne dcloop

	mrc p15, 0, SCRATCH0, c1, c0, 0
	orr SCRATCH0, SCRATCH0, #(1 << 12) @ enable instruction cache
	bic SCRATCH0, SCRATCH0, #((1 << 2) | (1 << 0)) @ disable data cache and mmu
	mcr p15, 0, SCRATCH0, c1, c0, 0

	@ load some internal registers
	ldr UARTBASE, =REG_UART_BASE

	@ start printing
	adr PARAM0, dtb_str
	blx print_str

	ldr PARAM0, =DTB_ADDRESS
	blx print_hex

	adr PARAM0, ker_str
	blx print_str

	ldr PARAM0, =KERNEL_ADDRESS
	blx print_hex

	adr PARAM0, boot_str
	blx print_str

	@ R0 is always zero
	@ R1 is the machine type (all ones to match none)
	@ R2 is a pointer to the physical address of the device tree
	mov r0, #0
	mov r1, #0xFFFFFFFF
	ldr r2, =DTB_ADDRESS
	ldr pc, =KERNEL_ADDRESS
.endfunc

.align 2
.pool

@ no need to align the strings
dtb_str: .asciz "DTB @ "
ker_str: .asciz "\n\rKernel @ "
boot_str: .asciz "\n\rBooting...\n\r"

.thumb

.func print_str, print_str
.align 1
1:
	ldrb SCRATCH1, [UARTBASE, #REG_UART_STAT]
	lsl SCRATCH1, #(32 - UART_STAT_TX_BUSY)
	bmi 1b
	strb SCRATCH0, [UARTBASE, #REG_UART_FIFO]

print_str:
	ldrb SCRATCH0, [PARAM0]
	add PARAM0, #1
	cmp SCRATCH0, #0
	bne 1b
	bx lr
.endfunc

.func print_hex, print_hex
.align 1
1: @ main print routine, SCRATCH0 should be a char '0'..'9', 'A'..'F'
	ldrb SCRATCH1, [UARTBASE, #REG_UART_STAT]
	lsl SCRATCH1, #(32 - UART_STAT_TX_BUSY)
	bmi 1b
	strb SCRATCH0, [UARTBASE, #REG_UART_FIFO]

	sub SCRATCH3, #1
	beq 4f @ branch out if nothing remains
	@ fall through to next char otherwise

2:
	lsr SCRATCH0, PARAM0, #28
	lsl PARAM0, #4
	@ extract the top nibble
	@ and shift up the input for the next interation

	cmp SCRATCH0, #10
	bhs 3f

	add SCRATCH0, #'0'
	b 1b

3: @ compensate for 0xA..0xF
	add SCRATCH0, #('A' - 10)
	b 1b

4:
	bx lr

print_hex:
	mov SCRATCH3, #8
	b 2b
.endfunc
