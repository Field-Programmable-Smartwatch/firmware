CROSS_PATH = ../compiler/cross/bin/
CROSS_PREFIX = $(CROSS_PATH)arm-none-eabi-
GCC = $(CROSS_PREFIX)gcc
LD = $(CROSS_PREFIX)ld
OBJCOPY =$(CROSS_PREFIX)objcopy

CPU = cortex-m4
INCLUDE = -Iinclude $(foreach inc_path, $(wildcard drivers/*), -I$(inc_path)) -Isrc
CFLAGS = -Wall -Werror -c -ffreestanding -nostdlib -mcpu=$(CPU) $(INCLUDE) -MMD -MF $(DEPDIR)/$*.d
LDFLAGS = -static -T linker.ld 

DEPDIR = .deps/

SOURCE = $(wildcard drivers/*/*.c) \
         $(wildcard src/*.c)

OBJECTS = $(patsubst %.c,%.o,$(SOURCE)) cp850-8x16.o

DEPENDS = $(patsubst %.c,$(DEPDIR)/%.d,$(SOURCE))


all: fpsw.bin

%.d:
	@mkdir -p $(@D)

%.o: %.psfu
	$(OBJCOPY) -O elf32-littlearm -B arm -I binary $^ $@

%.o: %.c Makefile
	$(GCC) $(CFLAGS) -o $@ $<

fpsw.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.bin: %.elf
	$(OBJCOPY) $^ -O binary $@

.PHONY: flash-%
flash-%:
	dfu-util -a 0 -i 0 -s 0x08000000:leave -D $*.bin

.PHONY: clean
clean:
	rm -rf $(DEPDIR) $(led-blink_OBJECTS) $(lpuart_OBJECTS) $(spi_OBJECTS) *.elf *.bin

include $(DEPENDS)
