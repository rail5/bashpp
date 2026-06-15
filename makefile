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

test-lsp: bin/bpp bin/bpp-lsp
	bin/bpp -Istdlib/ test-suite/lsp-semantic-tokens.bpp bin/bpp-lsp

vscode:
	@cd vscode && $(MAKE) --no-print-directory

clean-vscode:
	@cd vscode && $(MAKE) --no-print-directory clean
	@echo "Cleaned up VSCode extension files."

zed:
	cargo build --manifest-path zed/Cargo.toml --target wasm32-wasip1

test-zed:
	cargo check --manifest-path zed/Cargo.toml --target wasm32-wasip1

clean-zed:
	rm -rf zed/target zed/extension.wasm

clean: clean-flexbison clean-lsp clean-meta clean-objects clean-bin clean-std clean-manpages clean-technical-docs clean-vscode clean-zed

.PHONY: all test test-lsp vscode clean-vscode zed test-zed clean-zed

ifeq ($(filter clean%,$(MAKECMDGOALS)),)
-include $(shell find bin -name '*.d' 2>/dev/null)
endif
