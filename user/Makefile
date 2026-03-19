TARGET  ?= x86_64-elf
CC      := $(TARGET)-gcc
CFLAGS  := -std=c11 -O2 -g -ffreestanding -fno-pic -fno-pie -mno-red-zone \
           -Wall -Wextra -Werror -nostdlib -static
BUILD   := build

.PHONY: all clean

all: $(BUILD)/hello.elf $(BUILD)/shell.elf

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/hello.elf: hello.c link.ld | $(BUILD)
	$(CC) $(CFLAGS) -T link.ld hello.c -o $@

$(BUILD)/shell.elf: shell.c link.ld | $(BUILD)
	$(CC) $(CFLAGS) -T link.ld shell.c -o $@

clean:
	rm -rf $(BUILD)
