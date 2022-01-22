export MCU = stm32wb55xx
export SEAMOS_LIB = $(PWD)/SeamOS.a
export CONFIG = $(PWD)/kernel/config
SEAMOS_PATH = ../../
GCC = $(COMPILER)gcc
LD = $(COMPILER)ld
OBJCOPY = $(COMPILER)objcopy

include $(SEAMOS_PATH)/mcu/$(MCU)/config.mk

INCLUDE = -I$(SEAMOS_PATH) -I$(SEAMOS_PATH)/mcu/$(MCU)/include -I$(PWD)
CFLAGS ?= -Wall -Werror -c -ffreestanding -nostdlib $(MCU_CFLAGS) $(INCLUDE) -g \
          -DLOG_LEVEL=$(CONFIG_LOG_LEVEL) -MMD -MF $(DEPDIR)/$*.d
LDFLAGS ?= --omagic -static

DEPDIR = .deps/

KERNEL_LINKER_SCRIPT = ./kernel/linker.ld
KERNEL_SOURCE = $(wildcard ./kernel/*.c) $(wildcard ./kernel/*/*.c)
KERNEL_OBJECTS = $(patsubst %.c,%.o,$(KERNEL_SOURCE))

LIB_SOURCE = $(wildcard ./libraries/*.c) $(wildcard $(SEAMOS_PATH)/libraries/*.c)
LIB_OBJECTS = $(patsubst %.c,%.o,$(LIB_SOURCE))

APP_LINKER_SCRIPT = ./applications/linker.ld
HELLO_APP_SOURCE = $(wildcard ./applications/hello_world/*.c)
HELLO_APP_OBJECTS = $(patsubst %.c,%.o,$(HELLO_APP_SOURCE))

DEPENDS = $(patsubst %.c,$(DEPDIR)/%.d,$(KERNEL_SOURCE)) \
          $(patsubst %.c,$(DEPDIR)/%.d,$(LIB_SOURCE)) \
          $(patsubst %.c,$(DEPDIR)/%.d,$(HELLO_APP_SOURCE))



.SECONDARY: kernel.elf hello_app.elf
all: kernel.bin lib.a hello_app.bin

.PHONY: $(SEAMOS_LIB)
$(SEAMOS_LIB):
	$(MAKE) -C $(SEAMOS_PATH)

%.d:
	@mkdir -p $(@D)

%.o: %.c Makefile
	$(GCC) $(CFLAGS) -o $@ $<

kernel.elf: $(SEAMOS_LIB) $(KERNEL_OBJECTS) $(OS_LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -T $(KERNEL_LINKER_SCRIPT) -o $@ $(KERNEL_OBJECTS) $(SEAMOS_LIB)

lib.a: $(LIB_OBJECTS)
	$(AR) -rc $@ $^

hello_app.elf: $(HELLO_APP_OBJECTS) lib.a $(APP_LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -T $(APP_LINKER_SCRIPT) -o $@ $(HELLO_APP_OBJECTS) lib.a

%.bin: %.elf
	$(OBJCOPY) $^ -O binary $@

.PHONY: clean
clean:
	rm -rf $(DEPDIR) $(KERNEL_OBJECTS) $(LIB_OBJECTS) $(HELLO_APP_OBJECTS) *.bin *.elf *.a

.PHONY: flash
flash:
	st-flash write hello_app.bin 0x08010000
	st-flash write kernel.bin 0x08000000

include $(DEPENDS)
