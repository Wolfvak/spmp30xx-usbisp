#!/bin/sh
arm-none-eabi-gcc -Wall -nostdlib -Ttext=0x24000000 clkfreq.S -o clkfreq.elf
arm-none-eabi-objcopy -O binary clkfreq.elf clkfreq.bin
