TARGET 	:= x86_64-elf
CC		:= $(TARGET)-gcc
LD		:= $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy

CFLAGS  := -std=c11 -02 -g -ffreestanding -fno-pic -fno-pie -mno-red-zone \
		   -Wall -Wextra -Werror
LDFLAGS := -nostdlib -T linker.ld

BUILD	:= build
ISO_DIR := iso
KERNEL  := $(BUILD)/kernel.elf

SRCS_C  := src/kernel.c src/serial.c
SRCS_S  := src/boot.S
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS_C)) \
		   $(patsubst src/%.S,$(BUILD)/%.o,$(SRCS_S))

.PHONY: all clean iso run debug

all: $(KERNEL)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

iso: $(KERNEL)
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/kernel.ison $(ISO_DIR) >/dev/null

run: iso
	qemu-system-x86_64 \
		-cdrom $(BUILD)/kernel.iso \
		-serial stdio \
		-no-reboot -no-shutdown

debug: iso
	qemu-system-x86_64 \
		-cdrom $(BUILD)/kernel.iso \
		-serial stdio \
		-no-reboot -no-shutdown \
		-s -S
clean:
	rm -rf $(BUILD) $(ISO_DIR)