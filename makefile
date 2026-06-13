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
	bin/bpp -Istdlib/ test-suite/run.bpp

vscode:
	@cd vscode && $(MAKE) --no-print-directory

tree-sitter:
	@$(MAKE) -C tree-sitter-bashpp --no-print-directory generate

test-tree-sitter:
	@$(MAKE) -C tree-sitter-bashpp --no-print-directory test

clean-vscode:
	@cd vscode && $(MAKE) --no-print-directory clean
	@echo "Cleaned up VSCode extension files."

clean-tree-sitter:
	@$(MAKE) -C tree-sitter-bashpp --no-print-directory clean

clean: clean-flexbison clean-lsp clean-meta clean-objects clean-bin clean-std clean-manpages clean-technical-docs clean-vscode clean-tree-sitter

.PHONY: all test vscode tree-sitter test-tree-sitter clean-vscode clean-tree-sitter

ifeq ($(filter clean%,$(MAKECMDGOALS)),)
-include $(shell find bin -name '*.d' 2>/dev/null)
endif
