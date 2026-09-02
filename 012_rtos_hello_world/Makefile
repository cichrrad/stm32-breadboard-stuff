OUT_NAME=rtos_hello_world

SRCS := $(wildcard *.c) \
        $(shell find ../Source -name "*.c") \
        $(shell find ./freeRTOS -name "*.c") \

INC_DIRS := . \
			$(shell find ../Include -type d) \
			$(shell find ./freeRTOS -type d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostartfiles -mthumb $(INC_FLAGS) -T linker.ld -g -O2 --specs=nano.specs --specs=nosys.specs $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"