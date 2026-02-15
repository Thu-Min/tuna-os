TARGET  ?= x86_64-elf
CC		:= $(TARGET)-gcc
LD		:= $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy
GRUB_MKRESCUE ?= $(firstword \
	$(shell command -v x86_64-elf-grub-mkrescue 2>/dev/null) \
	$(shell command -v grub-mkrescue 2>/dev/null))
QEMU ?= qemu-system-x86_64
QEMU_FLAGS := -cdrom $(BUILD)/kernel.iso -display none -monitor none \
			  -serial stdio -no-reboot -no-shutdown

CFLAGS  := -std=c11 -O2 -g -ffreestanding -fno-pic -fno-pie -mno-red-zone \
		   -Wall -Wextra -Werror
LDFLAGS := -nostdlib -T linker.ld

BUILD	:= build
ISO_DIR := iso
KERNEL  := $(BUILD)/kernel.elf

SRCS_C  := src/kernel.c src/serial.c
SRCS_S  := src/boot.S
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS_C)) \
		   $(patsubst src/%.S,$(BUILD)/%.o,$(SRCS_S))

.PHONY: all clean iso run debug toolchain-check run-check

all: toolchain-check $(KERNEL)

toolchain-check:
	@missing=0; \
	command -v $(CC) >/dev/null || { \
		echo "error: missing compiler '$(CC)'"; \
		echo "install with: brew install x86_64-elf-binutils x86_64-elf-gcc"; \
		missing=1; \
	}; \
	test -n "$(GRUB_MKRESCUE)" || { \
		echo "error: missing grub-mkrescue"; \
		echo "install with: brew install x86_64-elf-grub"; \
		missing=1; \
	}; \
	command -v xorriso >/dev/null || { \
		echo "error: missing xorriso"; \
		echo "install with: brew install xorriso"; \
		missing=1; \
	}; \
	command -v mformat >/dev/null || { \
		echo "error: missing mtools (mformat)"; \
		echo "install with: brew install mtools"; \
		missing=1; \
	}; \
	test $$missing -eq 0

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

iso: toolchain-check $(KERNEL)
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(BUILD)/kernel.iso $(ISO_DIR) >/dev/null

run-check:
	@missing=0; \
	command -v $(QEMU) >/dev/null || { \
		echo "error: missing qemu-system-x86_64"; \
		echo "install with: brew install qemu"; \
		missing=1; \
	}; \
	test $$missing -eq 0

run: iso run-check
	$(QEMU) $(QEMU_FLAGS)

debug: iso run-check
	$(QEMU) $(QEMU_FLAGS) -s -S
clean:
	rm -rf $(BUILD) $(ISO_DIR)
