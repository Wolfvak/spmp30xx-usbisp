.cpu arm926ej-s

.arm

.set UART_FIFO, 0x22
.set UART_STAT, 0x2A
.set UART_TX_BUSY, 2

.align 2

.section .text

.global _start
_start:
	ldr r12, uart_base

prmain:
	adr r0, test_string

	prloop:
		ldrb r1, [r0], #1
		cmp r1, #0
		beq prmain

	txbusy:
		ldrb r11, [r12, #UART_STAT]
		tst r11, #UART_TX_BUSY
		bne txbusy

		strb r1, [r12, #UART_FIFO]
		b prloop

_pool:
.align 2
uart_base: .word 0x10001800

.align 2
test_string: .ascii "Hello, world!\n"
