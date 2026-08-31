OUT_NAME=bootloader_iap

SRCS := $(wildcard *.c) \
        $(shell find ../../Source -name "*.c") \
        $(shell find ./local_source -name "*.c") \

INC_DIRS := $(shell find ../../Include -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

build:
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb $(INC_FLAGS) -T linker.ld -g -O2 -nostdlib -fno-builtin -ffreestanding $(SRCS) -o ${OUT_NAME}.elf
flash:
	openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${OUT_NAME}.elf verify reset exit"

send-bin-1:
	stty -F /dev/ttyACM0 115200 raw -echo
	cat ../update_app/update_app1.bin > /dev/ttyACM0

send-bin-2:
	stty -F /dev/ttyACM0 115200 raw -echo
	cat ../update_app/update_app2.bin > /dev/ttyACM0
	
send-bin-3:
	stty -F /dev/ttyACM0 115200 raw -echo
	cat ../update_app/update_app3.bin > /dev/ttyACM0	