# Default to using all available CPU cores for parallel builds
# unless the user specifies a different number of jobs with -jN
ifeq ($(filter -j%,$(MAKEFLAGS)),)
	MAKEFLAGS += -j$(shell nproc)
endif

all: bin/bpp bin/bpp-lsp std
	@:

include mk/build.mk
include mk/stdlib.mk
include mk/docs.mk

test:
	bin/bpp test-suite/run.bpp

vscode:
	@cd vscode && $(MAKE) --no-print-directory

clean-vscode:
	@cd vscode && $(MAKE) --no-print-directory clean
	@echo "Cleaned up VSCode extension files."

clean: clean-antlr clean-lsp clean-meta clean-objects clean-bin clean-std clean-manual clean-technical-docs clean-vscode

.PHONY: all test vscode clean-vscode

ifeq ($(filter clean%,$(MAKECMDGOALS)),)
-include $(shell find bin -name '*.d' 2>/dev/null)
endif
