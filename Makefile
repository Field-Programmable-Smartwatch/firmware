include config.mk

INCLUDE = -I$(PWD) -Isrc -Ibootloader -I$(SEAMOS_PATH) -I$(SEAMOS_PATH)/mcu/$(MCU)/include \
          $(foreach inc_path, $(wildcard applications/*), -I$(inc_path))
CFLAGS = -Wall -Werror -c -ffreestanding -nostdlib -nolibc -mcpu=$(CPU) $(INCLUDE) \
         -DLOG_LEVEL=LOG_LEVEL_INFO -MMD -MF $(DEPDIR)/$*.d
LDFLAGS = --omagic -static

DEPDIR = .deps/

BOOTLOADER_SOURCE = $(wildcard bootloader/*.c)

SOURCE = $(wildcard src/*.c) \
         $(wildcard applications/*/*.c) \

BOOTLOADER_OBJECTS = $(patsubst %.c,%.o,$(BOOTLOADER_SOURCE))

OBJECTS = $(patsubst %.c,%.o,$(SOURCE)) cp850-8x16.o

DEPENDS = $(patsubst %.c,$(DEPDIR)/%.d,$(BOOTLOADER_SOURCE)) \
          $(patsubst %.c,$(DEPDIR)/%.d,$(SOURCE))


all: bootloader.bin fpsw.bin

.PHONY: $(SEAMOS_LIB)
$(SEAMOS_LIB):
	$(MAKE) -C $(SEAMOS_PATH)

%.d:
	@mkdir -p $(@D)

%.o: %.psfu
	$(OBJCOPY) -O elf32-littlearm -B arm -I binary $^ $@

%.o: %.c Makefile
	$(GCC) $(CFLAGS) -o $@ $<

fpsw.elf: $(SEAMOS_LIB) $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJECTS) $(SEAMOS_LIB)

bootloader.elf: $(SEAMOS_LIB) $(BOOTLOADER_OBJECTS) bootloader-linker.ld
	$(LD) $(LDFLAGS) -T bootloader-linker.ld -o $@ $(BOOTLOADER_OBJECTS) $(SEAMOS_LIB)

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
	rm -rf $(DEPDIR) $(BOOTLOADER_OBJECTS) $(OBJECTS) *.elf *.bin *.a
	$(MAKE) -C $(SEAMOS_PATH) clean

include $(DEPENDS)
