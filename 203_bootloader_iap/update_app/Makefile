OUT_NAME=update_app

SRCS := $(shell find ../../Sources/SourceBM -name "*.c") \
        $(shell find ../../Sources/Common -name "*.c") \
        $(shell find ./local_source -name "*.c")

INC_DIRS := $(shell find ../../Includes/IncludeBM -type d) \
			$(shell find ../../Includes/Common -type d)
			
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build-1:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -Os -nostdlib -fno-builtin -ffreestanding  app1.c startup.c $(SRCS) -o ${OUT_NAME}1.elf
	arm-none-eabi-objcopy -O binary ${OUT_NAME}1.elf ${OUT_NAME}1.bin
flash-1:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}1.elf verify reset exit"

build-2:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -Os -nostdlib -fno-builtin -ffreestanding  app2.c startup.c $(SRCS) -o ${OUT_NAME}2.elf
	arm-none-eabi-objcopy -O binary ${OUT_NAME}2.elf ${OUT_NAME}2.bin
flash-2:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}2.elf verify reset exit"

build-3:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -Os -nostdlib -fno-builtin -ffreestanding  app3.c startup.c $(SRCS) -o ${OUT_NAME}3.elf
	arm-none-eabi-objcopy -O binary ${OUT_NAME}3.elf ${OUT_NAME}3.bin
flash-3:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}3.elf verify reset exit"