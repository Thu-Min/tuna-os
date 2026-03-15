.PHONY: all iso run run-efi debug debug-efi clean

all iso run run-efi debug debug-efi clean:
	$(MAKE) -C kernel $@
