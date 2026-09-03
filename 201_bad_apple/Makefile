OUT_NAME=bad_apple

SRCS := $(wildcard *.c) \
		$(shell find ../Sources/Common -name "*.c") \
        ../Sources/SourceBM/device_drivers/gpio/*.c \
		../Sources/SourceBM/device_drivers/tick_engine/*.c

INC_DIRS := $(shell find ../Includes/IncludeBM -type d) \
			$(shell find ../Includes/Common -type d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -O2 -nostdlib -fno-builtin -ffreestanding $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"

# FIRST TIME RUN THESE COMMANDS
# 	`python3 -m venv venv` to create venv
# 	`source venv/bin/activate` to enter venv
# 	when inside venv:
#	`pip install opencv-python-headless pyserial numpy`
#	Then you can run make stream when in the venv
stream:
	python3 streamer.py