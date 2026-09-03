OUT_NAME=i2c_sensor

SRCS := $(wildcard *.c) \
        $(shell find ../Sources/SourceBM -name "*.c") \
        $(shell find ../Sources/Common -name "*.c") \
        $(shell find ./local_source -name "*.c")

INC_DIRS := $(shell find ../Includes/IncludeBM -type d) \
			$(shell find ../Includes/Common -type d)
			
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -O2 -nostdlib -fno-builtin -ffreestanding $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"