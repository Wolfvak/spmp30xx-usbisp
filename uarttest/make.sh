#!/bin/sh
arm-none-eabi-gcc -Wall -nostdlib -Ttext=0x24000000 uarttest.S -o uarttest.elf
arm-none-eabi-objcopy -O binary uarttest.elf uarttest.bin
