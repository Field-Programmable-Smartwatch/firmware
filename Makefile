CROSS_PATH = ../compiler/cross/bin/
CROSS_PREFIX = $(CROSS_PATH)arm-none-eabi-
GCC = $(CROSS_PREFIX)gcc
LD = $(CROSS_PREFIX)ld
OBJCOPY =$(CROSS_PREFIX)objcopy

CPU = cortex-m4
INCLUDE = -Iinclude -Ilibraries -Isrc -Ibootloader \
          $(foreach inc_path, $(wildcard drivers/*), -I$(inc_path)) \
          $(foreach inc_path, $(wildcard applications/*), -I$(inc_path))
CFLAGS = -Wall -Werror -c -ffreestanding -nostdlib -mcpu=$(CPU) $(INCLUDE) \
         -DLOG_LEVEL=LOG_LEVEL_DEBUG -MMD -MF $(DEPDIR)/$*.d
LDFLAGS = --omagic -static

DEPDIR = .deps/

BOOTLOADER_SOURCE = $(wildcard drivers/*/*.c) \
                    $(wildcard bootloader/*.c) \
                    $(wildcard libraries/*.c)

SOURCE = $(wildcard drivers/*/*.c) \
         $(wildcard src/*.c) \
         $(wildcard libraries/*.c) \
         $(wildcard applications/*/*.c) \

BOOTLOADER_OBJECTS = $(patsubst %.c,%.o,$(BOOTLOADER_SOURCE))

OBJECTS = $(patsubst %.c,%.o,$(SOURCE)) cp850-8x16.o

DEPENDS = $(patsubst %.c,$(DEPDIR)/%.d,$(BOOTLOADER_SOURCE)) \
          $(patsubst %.c,$(DEPDIR)/%.d,$(SOURCE))


all: bootloader.bin fpsw.bin

%.d:
	@mkdir -p $(@D)

%.o: %.psfu
	$(OBJCOPY) -O elf32-littlearm -B arm -I binary $^ $@

%.o: %.c Makefile
	$(GCC) $(CFLAGS) -o $@ $<

fpsw.elf: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJECTS)

bootloader.elf: $(BOOTLOADER_OBJECTS) bootloader-linker.ld
	$(LD) $(LDFLAGS) -T bootloader-linker.ld -o $@ $(BOOTLOADER_OBJECTS)

%.bin: %.elf
	$(OBJCOPY) $^ -O binary $@

.PHONY: dd
dd:
	sudo dd if=fpsw.elf of=$(DEV) bs=512
	sudo rm $(DEV) # It seems I have to do this due to some linux bug

.PHONY: flash-%
flash-%:
	dfu-util -a 0 -i 0 -s 0x08000000:leave -D $*.bin

.PHONY: clean
clean:
	rm -rf $(DEPDIR) $(BOOTLOADER_OBJECTS) $(OBJECTS) *.elf *.bin

include $(DEPENDS)
