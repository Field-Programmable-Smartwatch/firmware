CROSS_PATH = ../../compiler/cross/bin/
CROSS_PREFIX = ${CROSS_PATH}arm-none-eabi-
GCC = ${CROSS_PREFIX}gcc
LD = ${CROSS_PREFIX}ld
OBJCOPY =${CROSS_PREFIX}objcopy

CPU = cortex-m4
DRIVER_PATH = ../drivers
INCLUDE = -I../include -I${DRIVER_PATH}/gpio/ -I${DRIVER_PATH}/lpuart/ -I${DRIVER_PATH}/spi -I../debug -I../libraries
CFLAGS = -Wall -Werror -c -ffreestanding -nostdlib -mcpu=${CPU} ${INCLUDE} -MMD
LDFLAGS = -static -T ../linker.ld 

SOURCES = $(wildcard *.c) $(wildcard ${DRIVER_PATH}/*/*.c) $(wildcard ../debug/*.c) $(wildcard ../libraries/*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPENDS = $(patsubst %.c,%.d,$(SOURCES))

all: spi.bin

clean:
	rm -f ${OBJECTS} ${DEPENDS} *.elf *.bin

-include ${DEPENDS}
%.o: %.c Makefile
	${GCC} ${CFLAGS} -o $@ $<

spi.elf: ${OBJECTS}
	${LD} ${LDFLAGS} -o $@ $^

spi.bin: spi.elf
	${OBJCOPY} $^ -O binary $@
