include mk/config.mk
include mk/objects.mk
include mk/generatedcode.mk

$(BINDIR)/bpp: $(BPP_OBJS) $(FLEXBISON_GENERATED_OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

$(BINDIR)/bpp-lsp: $(LSP_MAIN_OBJ) $(BPP_OBJS) $(FLEXBISON_GENERATED_OBJS) $(LSP_OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

clean-bin:
	@rm -f $(BINDIR)/bpp $(BINDIR)/bpp-lsp
	@echo "Cleaned up binaries."

.PHONY: clean-bin
