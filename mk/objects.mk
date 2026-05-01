# Generic compilation rules
#
# Mirror the src/ directory structure into the object directory and compile
# everything with a single pattern rule. Subtree-specific prerequisites are
# expressed via dependency-only pattern rules below.

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# The LSP generator is not part of the compiler itself.
# It's an intermediary tool that we use to generate classes for the LSP implementation
$(OBJDIR)/lsp/generator/%.o: $(SRCDIR)/lsp/generator/%.cpp $(LSPDIR)/metaModel.json
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Generated-code prerequisites
#
# We avoid per-subtree pattern rules; instead, we attach the generator stamps to
# the explicit object lists that need them.

# Flex/Bison-generated sources are prerequisites for everything, since the generated
# headers are included throughout the compiler and language server code.
$(BPP_OBJS) $(FLEXBISON_GENERATED_OBJS) $(MAIN_OBJ) $(LSP_MAIN_OBJ) $(LSP_OBJS): $(FLEXBISON_GENERATED_STAMP)

# LSP-generated sources are prerequisites for the language server objects and entrypoint
$(LSP_MAIN_OBJ) $(LSP_OBJS): $(LSP_GENERATED_STAMP)

# Both entrypoints depend on the version and updated year headers, which are generated from debian/changelog
$(MAIN_OBJ) $(LSP_MAIN_OBJ): $(SRCDIR)/version.h $(SRCDIR)/updated_year.h


clean-objects:
	@rm -rf $(OBJDIR)
	@find $(BINDIR) -name '*.d' -exec rm -f {} +
	@echo "Cleaned up object files."

.PHONY: clean-objects
