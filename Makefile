CROSS_PATH = ../compiler/cross/bin/
CROSS_PREFIX = ${CROSS_PATH}arm-none-eabi-
GCC = ${CROSS_PREFIX}gcc
LD = ${CROSS_PREFIX}ld
OBJCOPY =${CROSS_PREFIX}objcopy

CPU = cortex-m4
INCLUDE = -Iinclude -Ilibraries $(foreach inc_path, $(wildcard drivers/*), -I$(inc_path)) -Ilibraries/terminal
CFLAGS = -Wall -Werror -c -ffreestanding -nostdlib -mcpu=${CPU} ${INCLUDE} -MMD -MF ${DEPDIR}/$*.d
LDFLAGS = -static -T linker.ld 

DEPDIR = .deps/
COMMON_SOURCES = $(wildcard drivers/gpio/*.c) \
                 $(wildcard drivers/lpuart/*.c) \
                 $(wildcard libraries/*.c) \
                 $(wildcard boot/*.c)

led-blink_SOURCES = $(COMMON_SOURCES) $(wildcard led-blink-example/*.c)
led-blink_OBJECTS = $(patsubst %.c,%.o,$(led-blink_SOURCES))
led-blink_DEPENDS = $(patsubst %.c,${DEPDIR}/%.d,$(led-blink_SOURCES))

lpuart_SOURCES = $(COMMON_SOURCES) $(wildcard lpuart-example/*.c)
lpuart_OBJECTS = $(patsubst %.c,%.o,$(lpuart_SOURCES))
lpuart_DEPENDS = $(patsubst %.c,${DEPDIR}/%.d,$(lpuart_SOURCES))

spi_SOURCES = $(COMMON_SOURCES) $(wildcard spi-example/*.c) $(wildcard drivers/spi/*.c) \
              $(wildcard libraries/terminal/*.c) $(wildcard drivers/display/*.c)
spi_OBJECTS = $(patsubst %.c,%.o,$(spi_SOURCES)) cp850-8x16.o
spi_DEPENDS = $(patsubst %.c,${DEPDIR}/%.d,$(spi_SOURCES))

all: led-blink.bin lpuart.bin spi.bin

%.d:
	@mkdir -p $(@D)

%.o: %.psfu
	${OBJCOPY} -O elf32-littlearm -B arm -I binary $^ $@

%.o: %.c Makefile
	${GCC} ${CFLAGS} -o $@ $<

led-blink.elf: $(led-blink_OBJECTS)
	${LD} ${LDFLAGS} -o $@ $^

lpuart.elf: $(lpuart_OBJECTS)
	${LD} ${LDFLAGS} -o $@ $^

spi.elf: $(spi_OBJECTS)
	${LD} ${LDFLAGS} -o $@ $^

%.bin: %.elf
	${OBJCOPY} $^ -O binary $@

.PHONY: led-blink-example
led-blink-example: led-blink.bin

.PHONY: lpuart-example
lpuart-example: lpuart.bin

.PHONY: spi-example
spi-example: spi.bin

.PHONY: flash-%
flash-%:
	dfu-util -a 0 -i 0 -s 0x08000000:leave -D $*.bin

.PHONY: clean
clean:
	rm -rf ${DEPDIR} $(led-blink_OBJECTS) $(lpuart_OBJECTS) $(spi_OBJECTS) *.elf *.bin

include $(led-blink_DEPENDS) $(lpuart_DEPENDS) $(spi_DEPENDS)
