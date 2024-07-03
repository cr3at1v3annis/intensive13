ifdef SYSSRC
 KERNEL_SOURCES	 = $(SYSSRC)
else
 KERNEL_UNAME	:= $(shell uname -r)
 KERNEL_SOURCES	 = /lib/modules/$(KERNEL_UNAME)/build
endif


default: modules
.PHONY: default
install: modules_install
	update-initramfs -u

.PHONY: install


%::
	$(MAKE) -C $(KERNEL_SOURCES) \
        M=$$PWD $@
