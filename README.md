# spmp30xx-usbisp

tools to manipulate the spmp30xx when in USB ISP mode (aka "bootloader" or "recovery" modes)

requires libusb-1.0, seems to only work on linux systems


## spmp_isp

lets you arbitrarily upload and download data on memory and boot a payload (always loaded @ 0x24000000)

the sizes should be 256 byte aligned, or they will be padded with zeroes


## spmp_iorw

a simple tool akin to devmem that lets you peek and poke registers in the IO space (0x10000000 - 0x1000FFFF)


## spmp_playground

collection of headers with registers and other code that read and write to IO space in order to execute tests and gather other info from the system


### loaders

simple payloads that can be booted via spmp_isp, few of them are useful
