.PHONY: all iso run debug clean

all iso run debug clean:
	$(MAKE) -C kernel $@
