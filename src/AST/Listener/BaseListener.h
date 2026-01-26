/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <memory>
#include <type_traits>

#include <AST/ASTNode.h>
#include <AST/NodeTypes.h>
#include <AST/Nodes/Nodes.h>
#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace AST {

// Single source of truth for node list (used to generate wrappers + map)
#define AST_LISTENER_NODE_LIST(X) \
	X(ArrayAssignment) \
	X(ArrayIndex) \
	X(Bash53NativeSupershell) \
	X(BashArithmeticForCondition) \
	X(BashArithmeticForStatement) \
	X(BashArithmeticStatement) \
	X(BashArithmeticSubstitution) \
	X(BashCaseInput) \
	X(BashCasePattern) \
	X(BashCasePatternHeader) \
	X(BashCaseStatement) \
	X(BashCommand) \
	X(BashCommandSequence) \
	X(BashForStatement) \
	X(BashFunction) \
	X(BashIfCondition) \
	X(BashIfElseBranch) \
	X(BashIfRootBranch) \
	X(BashIfStatement) \
	X(BashInCondition) \
	X(BashPipeline) \
	X(BashRedirection) \
	X(BashSelectStatement) \
	X(BashTestConditionCommand) \
	X(BashUntilStatement) \
	X(BashVariable) \
	X(BashWhileOrUntilCondition) \
	X(BashWhileStatement) \
	X(Block) \
	X(ClassDefinition) \
	X(Connective) \
	X(ConstructorDefinition) \
	X(DatamemberDeclaration) \
	X(DeleteStatement) \
	X(DestructorDefinition) \
	X(DoublequotedString) \
	X(DynamicCast) \
	X(DynamicCastTarget) \
	X(HeredocBody) \
	X(HereString) \
	X(IncludeStatement) \
	X(MethodDefinition) \
	X(NewStatement) \
	X(ObjectAssignment) \
	X(ObjectInstantiation) \
	X(ObjectReference) \
	X(ParameterExpansion) \
	X(PointerDeclaration) \
	X(PrimitiveAssignment) \
	X(ProcessSubstitution) \
	X(Program) \
	X(RawSubshell) \
	X(RawText) \
	X(Rvalue) \
	X(SubshellSubstitution) \
	X(Supershell) \
	X(TypeofExpression) \
	X(ValueAssignment) \

/**
 * @class BaseListener
 * @brief CRTP base class for AST listeners.
 * CRTP is a kind of language hack that makes static polymorphism possible in C++.
 *
 * Instead of using runtime virtual dispatch and RTTI (which incur performance penalties),
 * the base class is declared as a template,
 * and the derived class is instantiated as `class Derived : public Base<Derived>`
 * (i.e., passes itself as the template parameter)
 *
 * Normally, a base class could never call a derived class's functions directly,
 * because those derived functions could not possibly have been declared at the time the base class was defined
 * (this results in a compiler error, the compiler says "what function's that then? I haven't heard of it yet").
 *
 * But doing things this way means that the base class is not as yet "defined" until
 * the derived class is defined (since the derived class is needed to instantiate the base class template).
 *
 * The result is that the base class can now call functions of the derived class directly, STATICALLY, at compile-time.
 *
 * This allows us to avoid the overhead of virtual function calls and RTTI lookups,
 * while still allowing the derived class to implement only the functions it cares about.
 *
 * If this is a bit confusing, that's fine.
 * Some people call CRTP an "advanced C++ technique" -- this is code for TOTAL HACK.
 * Hacky tricks like this are bound to be confusing.
 * The important thing is that it works.
 * 
 * @tparam Derived The derived listener class.
 */
template <class Derived>
class BaseListener {
	private:
		Derived& self() {
			return static_cast<Derived&>(*this);
		}

	protected:
		bool program_has_errors = false;

	public:
		virtual ~BaseListener() = default;

		void walk(std::shared_ptr<AST::ASTNode> node) {
			try {
				switch (node->getType()) {
					#define AST_CASE(Name) \
						case AST::NodeType::Name: \
							if constexpr (requires(Derived& d, std::shared_ptr<AST::Name> x) { d.enter##Name(x); }) { \
								self().enter##Name(std::static_pointer_cast<AST::Name>(node)); \
							} \
							for (const auto& child : node->getChildren()) { \
								walk(child); \
							} \
							if constexpr (requires(Derived& d, std::shared_ptr<AST::Name> x) { d.exit##Name(x); }) { \
								self().exit##Name(std::static_pointer_cast<AST::Name>(node)); \
							} \
							break;
					
					AST_LISTENER_NODE_LIST(AST_CASE)

					#undef AST_CASE

					default:
						throw bpp::ErrorHandling::InternalError("Unknown AST node type encountered in listener walk");
				}
			} catch (const bpp::ErrorHandling::SyntaxError& e) {
				// Cancel traversal of this node and its children, but continue to traverse the rest of the tree
				this->program_has_errors = true;
				e.print();
				return;
			}
		}
		#undef AST_LISTENER_NODE_LIST

		inline void set_has_errors(bool has_errors) {
			this->program_has_errors = has_errors;
		}
};

} // namespace AST
