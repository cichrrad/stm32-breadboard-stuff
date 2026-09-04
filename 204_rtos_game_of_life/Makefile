OUT_NAME=rtos_game_of_life

SRCS := $(wildcard *.c) \
        $(shell find ../Sources/SourceRTOS -name "*.c") \
        $(shell find ./local_source -name "*.c") \
        $(shell find ../Sources/Common -name "*.c") \
        $(shell find ../FreeRTOS -name "*.c")

INC_DIRS := . \
			$(shell find ../Includes/IncludeRTOS -type d) \
			$(shell find ../Includes/Common -type d) \
			$(shell find ./local_source -type d) \
			$(shell find ../FreeRTOS -type d)
			
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostartfiles -mthumb $(INC_FLAGS) -T linker.ld -g -O3 --specs=nano.specs --specs=nosys.specs $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"