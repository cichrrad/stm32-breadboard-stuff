OUT_NAME=hello_world

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -T linker.ld -g -O0 -nostdlib startup.c main.c -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"