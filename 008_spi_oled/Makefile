OUT_NAME=spi_oled
BAUD_RATE=115200

# Target specific files to dodge conflicts
SRCS := $(wildcard *.c) \
        $(shell find ../Sources/SourceBM/device_drivers/gpio -name "*.c") \
        $(shell find ../Sources/Common -name "*.c")

INC_DIRS := $(shell find ../Includes/IncludeBM/device_drivers/gpio -type d) \
			$(shell find ../Includes/Common -type d)
			
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -O0 -nostdlib $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"
