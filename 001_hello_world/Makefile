build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -T linker.ld -g -O0 -nostdlib startup.c main.c -o bare_metal.elf

uart:
	minicom -D /dev/ttyACM0 -b 115200