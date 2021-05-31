.cpu arm926ej-s

@ Compile with:
@ arm-none-eabi-gcc -Wall -nostdlib -Ttext=0x24000000 uarttest.S -o uarttest.elf
@ arm-none-eabi-objcopy -O binary uarttest.elf uarttest.bin

.arm

.align 2

.section .text

.set REG_ENABLE, 0x14
.set REG_FLAGS, 0x10
.set REG_COUNTER, 0x00

.global _start
_start:
	msr cpsr_c, #0xD3 @ disable all interrupts

	ldr r0, =0x10000120 @ CLOCKS

	ldr r11, =0x24C00000
	ldr r12, =0xDEAD7777
	str r12, [r11, #0]

	mov r1, #1
	strb r1, [r0, #0x02] @ HCLK?
	strb r1, [r0, #0x03] @ MCLK?
	strb r1, [r0, #0x12] @ BUS?
	strb r1, [r0, #0x11] @ AHB?

	mov r1, #0
	strb r1, [r0, #0x10] @ CPU?

	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #(1 << 12) @ enable instruction cache
	@orr r0, r0, #((1 << 2) | (1 << 0)) @ enable data cache and mmu
	mcr p15, 0, r0, c1, c0, 0

1: @ test, clean and invalidate all dcache
	mrc p15, 0, r15, c7, c14, 3
	bne 1b

	ldr r0, =0x10001030 @ COUNTER, FLAGS & ENABLE
	ldr r1, =0x10001318 @ DIVIDER

	mov r4, #0
	mov r5, #1 @ count-up flag
	strb r4, [r0, #REG_ENABLE] @ disable all timers

	strh r4, [r1] @ no divider, use the raw 12MHz clock
	str r4, [r0, #REG_COUNTER] @ reset counter to zero

	strb r4, [r0, #REG_FLAGS]
	strb r5, [r0, #REG_FLAGS] @ set count up (no reload)

	ldr r12, =100000 @ cycle counter register

	str r11, [r11, #4]

	strb r5, [r0, #REG_ENABLE] @ start counting cycles

2:
	subs r12, r12, #4
	bne 2b

	ldr r12, [r0, #REG_COUNTER]

	str r12, [r11, #0]
	ldr r12, =0xDEADBEEF
	str r12, [r11, #4]

3: @ test, clean and invalidate all dcache
	mrc p15, 0, r15, c7, c14, 3
	bne 3b

	mov r4, #0
	strb r4, [r0, #REG_ENABLE] @ disable all timers again

	b .

.pool

.align 8
