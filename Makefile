obj-m += redtidekm.o
OUT_DIR := $(PWD)/out
KERNEL_DIR := $(shell nix-build -E '(import <nixpkgs> {}).linux.dev' --no-out-link)

$(shell mkdir -p $(OUT_DIR))

all:
	$(MAKE) -C $(KERNEL_DIR)/lib/modules/*/build M=$(PWD) modules
	-mv -f *.ko *.mod *.mod.c *.mod.o *.o .*.cmd .tmp_versions Module.symvers modules.order $(OUT_DIR)/ 2>/dev/null || true

clean:
	$(MAKE) -C $(KERNEL_DIR)/lib/modules/*/build M=$(PWD) clean
	rm -rf $(OUT_DIR)
