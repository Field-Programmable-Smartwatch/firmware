CROSS_PREFIX = ../../compiler/cross/bin/arm-none-eabi-
GCC = ${CROSS_PREFIX}gcc
LD = ${CROSS_PREFIX}ld
OBJCOPY =${CROSS_PREFIX}objcopy

CPU = cortex-m4

CFLAGS = -c -ffreestanding -nostdlib -mcpu=${CPU} -I./include
LDFLAGS = -static -T linker.ld 

all: build link objcopy

clean:
	rm -f *.o *.elf *.bin


%.o: %.c
	${GCC} ${CFLAGS} -o $@ $^

build: main.o interrupt_handlers.o




led-blink.elf: main.o interrupt_handlers.o
	${LD} ${LDFLAGS} -o $@ $^ 

link: led-blink.elf




led_blink.bin: led-blink.elf
	${OBJCOPY} $^ -O binary $@

objcopy: led_blink.bin
