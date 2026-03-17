TARGET  ?= x86_64-elf
CC		:= $(TARGET)-gcc
LD		:= $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy
GRUB_MKRESCUE ?= $(firstword \
	$(shell command -v x86_64-elf-grub-mkrescue 2>/dev/null) \
	$(shell command -v grub-mkrescue 2>/dev/null))
QEMU ?= qemu-system-x86_64
QEMU_EFI_CODE ?= $(firstword \
	$(wildcard /opt/homebrew/share/qemu/edk2-x86_64-code.fd) \
	$(wildcard /usr/local/share/qemu/edk2-x86_64-code.fd))
QEMU_EFI_VARS_TEMPLATE ?= $(firstword \
	$(wildcard /opt/homebrew/share/qemu/edk2-x86_64-vars.fd) \
	$(wildcard /usr/local/share/qemu/edk2-x86_64-vars.fd) \
	$(wildcard /opt/homebrew/share/qemu/edk2-i386-vars.fd) \
	$(wildcard /usr/local/share/qemu/edk2-i386-vars.fd))

CFLAGS  := -std=c11 -O2 -g -ffreestanding -fno-pic -fno-pie -mno-red-zone \
		   -Wall -Wextra -Werror
LDFLAGS := -nostdlib -T linker.ld

BUILD	:= build
ISO_DIR := iso
KERNEL  := $(BUILD)/kernel.elf
QEMU_EFI_VARS := $(BUILD)/edk2-vars.fd
QEMU_COMMON_FLAGS = -boot order=d -cdrom $(BUILD)/kernel.iso -display none -monitor none \
					-serial stdio -no-reboot -no-shutdown
QEMU_BIOS_FLAGS = -machine q35,accel=tcg $(QEMU_COMMON_FLAGS)
QEMU_EFI_FLAGS = -machine q35,accel=tcg \
				 -drive if=pflash,format=raw,readonly=on,file=$(QEMU_EFI_CODE) \
				 -drive if=pflash,format=raw,file=$(QEMU_EFI_VARS) \
				 $(QEMU_COMMON_FLAGS)

SRCS_C  := src/kernel.c src/serial.c src/gdt.c src/idt.c src/pic.c src/pit.c src/multiboot2.c src/pmm.c src/vmm.c src/kheap.c
SRCS_S  := src/boot.S src/gdt_flush.S src/interrupts.S
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS_C)) \
		   $(patsubst src/%.S,$(BUILD)/%.o,$(SRCS_S))

.PHONY: all clean iso run run-bios debug debug-bios toolchain-check run-check efi-check

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

efi-check:
	@missing=0; \
	test -n "$(QEMU_EFI_CODE)" || { \
		echo "error: missing QEMU EDK2 code firmware"; \
		echo "expected: /opt/homebrew/share/qemu/edk2-x86_64-code.fd"; \
		missing=1; \
	}; \
	test -n "$(QEMU_EFI_VARS_TEMPLATE)" || { \
		echo "error: missing QEMU EDK2 vars template"; \
		echo "expected one of: edk2-x86_64-vars.fd or edk2-i386-vars.fd"; \
		missing=1; \
	}; \
	test $$missing -eq 0

$(QEMU_EFI_VARS): efi-check | $(BUILD)
	@if [ ! -f "$@" ]; then cp "$(QEMU_EFI_VARS_TEMPLATE)" "$@"; fi

run: iso run-check efi-check $(QEMU_EFI_VARS)
	$(QEMU) $(QEMU_EFI_FLAGS)

run-bios: iso run-check
	$(QEMU) $(QEMU_BIOS_FLAGS)

debug: iso run-check efi-check $(QEMU_EFI_VARS)
	$(QEMU) $(QEMU_EFI_FLAGS) -s -S

debug-bios: iso run-check
	$(QEMU) $(QEMU_BIOS_FLAGS) -s -S
clean:
	rm -rf $(BUILD) $(ISO_DIR)
