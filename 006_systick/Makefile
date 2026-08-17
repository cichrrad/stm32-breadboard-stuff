OUT_NAME=systick_blinky
BAUD_RATE=115200

SRCS := $(wildcard *.c) \
        $(shell find ../Source -name "*.c")

INC_DIRS := $(shell find ../Include -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -O0 -nostdlib $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"
