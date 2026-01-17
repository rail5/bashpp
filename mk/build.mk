include mk/config.mk
include mk/objects.mk
include mk/generatedcode.mk

$(BINDIR)/bpp: $(BPP_OBJS) $(FLEXBISON_OBJS) $(LISTENER_OBJS) $(COMPILER_HANDLERS_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^

$(BINDIR)/bpp-lsp: $(LSP_MAIN_OBJ) $(BPP_OBJS) $(FLEXBISON_OBJS) $(LISTENER_OBJS) $(COMPILER_HANDLERS_OBJS) $(EXTRA_OBJS) $(LSP_OBJS) $(LSP_HANDLERS_OBJS) $(LSP_INCLUDE_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^

clean-bin:
	@rm -f $(BINDIR)/bpp $(BINDIR)/bpp-lsp
	@echo "Cleaned up binaries."

.PHONY: clean-bin
