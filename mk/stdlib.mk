# RULES FOR BUILDING THE STANDARD LIBRARY

.SUFFIXES:

# STDLIB_FILES:
# Find all files without extensions in the stdlib directory
# And append ".sh" to each file name
STDLIB_FILES := $(shell find stdlib -type f -not -name '*.*' -exec basename {} \; | sed 's/^/stdlib\//; s/$$/.sh/')

std: $(STDLIB_FILES)

stdlib/%.sh: stdlib/% bin/bpp
	@echo "Compiling stdlib: $<"
	@bin/bpp -o $@ $<

clean-std:
	@rm -f stdlib/*.sh
	@echo "Cleaned up stdlib files."

.PHONY: std clean-std
