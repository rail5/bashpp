/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BaseListener.h"
#include "../../internal_error.h"

/**
 * @brief Walk the AST starting from the given node, calling enter and exit methods for each node.
 * This method dispatches to the appropriate enter and exit methods based on the node type,
 * walking the tree in a depth-first manner.
 * 
 * @param node The AST node to start walking from.
 */
void AST::BaseListener::walk(std::shared_ptr<AST::ASTNode> node) {
	if (node == nullptr) return;

	auto it = enterExitMap.find(node->getType());
	if (it == enterExitMap.end()) {
		throw internal_error("No enter/exit functions defined for node type " + std::to_string(static_cast<int>(node->getType())));
	}

	auto [enterFunc, exitFunc] = it->second;
	std::invoke(enterFunc, this, node);

	for (const auto& child : node->getChildren()) {
		walk(child);
	}

	std::invoke(exitFunc, this, node);
}

void AST::BaseListener::_enterArrayIndex(std::shared_ptr<AST::ASTNode> node) {
	enterArrayIndex(std::dynamic_pointer_cast<AST::ArrayIndex>(node));
}
void AST::BaseListener::_exitArrayIndex(std::shared_ptr<AST::ASTNode> node) {
	exitArrayIndex(std::dynamic_pointer_cast<AST::ArrayIndex>(node));
}
void AST::BaseListener::_enterBashArithmeticForCondition(std::shared_ptr<AST::ASTNode> node) {
	enterBashArithmeticForCondition(std::dynamic_pointer_cast<AST::BashArithmeticForCondition>(node));
}
void AST::BaseListener::_exitBashArithmeticForCondition(std::shared_ptr<AST::ASTNode> node) {
	exitBashArithmeticForCondition(std::dynamic_pointer_cast<AST::BashArithmeticForCondition>(node));
}
void AST::BaseListener::_enterBashArithmeticForStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashArithmeticForStatement(std::dynamic_pointer_cast<AST::BashArithmeticForStatement>(node));
}
void AST::BaseListener::_exitBashArithmeticForStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashArithmeticForStatement(std::dynamic_pointer_cast<AST::BashArithmeticForStatement>(node));
}
void AST::BaseListener::_enterBashArithmeticStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashArithmeticStatement(std::dynamic_pointer_cast<AST::BashArithmeticStatement>(node));
}
void AST::BaseListener::_exitBashArithmeticStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashArithmeticStatement(std::dynamic_pointer_cast<AST::BashArithmeticStatement>(node));
}
void AST::BaseListener::_enterBashArithmeticSubstitution(std::shared_ptr<AST::ASTNode> node) {
	enterBashArithmeticSubstitution(std::dynamic_pointer_cast<AST::BashArithmeticSubstitution>(node));
}
void AST::BaseListener::_exitBashArithmeticSubstitution(std::shared_ptr<AST::ASTNode> node) {
	exitBashArithmeticSubstitution(std::dynamic_pointer_cast<AST::BashArithmeticSubstitution>(node));
}
void AST::BaseListener::_enterBashCaseInput(std::shared_ptr<AST::ASTNode> node) {
	enterBashCaseInput(std::dynamic_pointer_cast<AST::BashCaseInput>(node));
}
void AST::BaseListener::_exitBashCaseInput(std::shared_ptr<AST::ASTNode> node) {
	exitBashCaseInput(std::dynamic_pointer_cast<AST::BashCaseInput>(node));
}
void AST::BaseListener::_enterBashCasePatternAction(std::shared_ptr<AST::ASTNode> node) {
	enterBashCasePatternAction(std::dynamic_pointer_cast<AST::BashCasePatternAction>(node));
}
void AST::BaseListener::_exitBashCasePatternAction(std::shared_ptr<AST::ASTNode> node) {
	exitBashCasePatternAction(std::dynamic_pointer_cast<AST::BashCasePatternAction>(node));
}
void AST::BaseListener::_enterBashCasePattern(std::shared_ptr<AST::ASTNode> node) {
	enterBashCasePattern(std::dynamic_pointer_cast<AST::BashCasePattern>(node));
}
void AST::BaseListener::_exitBashCasePattern(std::shared_ptr<AST::ASTNode> node) {
	exitBashCasePattern(std::dynamic_pointer_cast<AST::BashCasePattern>(node));
}
void AST::BaseListener::_enterBashCasePatternHeader(std::shared_ptr<AST::ASTNode> node) {
	enterBashCasePatternHeader(std::dynamic_pointer_cast<AST::BashCasePatternHeader>(node));
}
void AST::BaseListener::_exitBashCasePatternHeader(std::shared_ptr<AST::ASTNode> node) {
	exitBashCasePatternHeader(std::dynamic_pointer_cast<AST::BashCasePatternHeader>(node));
}
void AST::BaseListener::_enterBashCaseStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashCaseStatement(std::dynamic_pointer_cast<AST::BashCaseStatement>(node));
}
void AST::BaseListener::_exitBashCaseStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashCaseStatement(std::dynamic_pointer_cast<AST::BashCaseStatement>(node));
}
void AST::BaseListener::_enterBashCommand(std::shared_ptr<AST::ASTNode> node) {
	enterBashCommand(std::dynamic_pointer_cast<AST::BashCommand>(node));
}
void AST::BaseListener::_exitBashCommand(std::shared_ptr<AST::ASTNode> node) {
	exitBashCommand(std::dynamic_pointer_cast<AST::BashCommand>(node));
}
void AST::BaseListener::_enterBashCommandSequence(std::shared_ptr<AST::ASTNode> node) {
	enterBashCommandSequence(std::dynamic_pointer_cast<AST::BashCommandSequence>(node));
}
void AST::BaseListener::_exitBashCommandSequence(std::shared_ptr<AST::ASTNode> node) {
	exitBashCommandSequence(std::dynamic_pointer_cast<AST::BashCommandSequence>(node));
}
void AST::BaseListener::_enterBashForStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashForStatement(std::dynamic_pointer_cast<AST::BashForStatement>(node));
}
void AST::BaseListener::_exitBashForStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashForStatement(std::dynamic_pointer_cast<AST::BashForStatement>(node));
}
void AST::BaseListener::_enterBashFunction(std::shared_ptr<AST::ASTNode> node) {
	enterBashFunction(std::dynamic_pointer_cast<AST::BashFunction>(node));
}
void AST::BaseListener::_exitBashFunction(std::shared_ptr<AST::ASTNode> node) {
	exitBashFunction(std::dynamic_pointer_cast<AST::BashFunction>(node));
}
void AST::BaseListener::_enterBashIfCondition(std::shared_ptr<AST::ASTNode> node) {
	enterBashIfCondition(std::dynamic_pointer_cast<AST::BashIfCondition>(node));
}
void AST::BaseListener::_exitBashIfCondition(std::shared_ptr<AST::ASTNode> node) {
	exitBashIfCondition(std::dynamic_pointer_cast<AST::BashIfCondition>(node));
}
void AST::BaseListener::_enterBashIfElseBranch(std::shared_ptr<AST::ASTNode> node) {
	enterBashIfElseBranch(std::dynamic_pointer_cast<AST::BashIfElseBranch>(node));
}
void AST::BaseListener::_exitBashIfElseBranch(std::shared_ptr<AST::ASTNode> node) {
	exitBashIfElseBranch(std::dynamic_pointer_cast<AST::BashIfElseBranch>(node));
}
void AST::BaseListener::_enterBashIfRootBranch(std::shared_ptr<AST::ASTNode> node) {
	enterBashIfRootBranch(std::dynamic_pointer_cast<AST::BashIfRootBranch>(node));
}
void AST::BaseListener::_exitBashIfRootBranch(std::shared_ptr<AST::ASTNode> node) {
	exitBashIfRootBranch(std::dynamic_pointer_cast<AST::BashIfRootBranch>(node));
}
void AST::BaseListener::_enterBashIfStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashIfStatement(std::dynamic_pointer_cast<AST::BashIfStatement>(node));
}
void AST::BaseListener::_exitBashIfStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashIfStatement(std::dynamic_pointer_cast<AST::BashIfStatement>(node));
}
void AST::BaseListener::_enterBashInCondition(std::shared_ptr<AST::ASTNode> node) {
	enterBashInCondition(std::dynamic_pointer_cast<AST::BashInCondition>(node));
}
void AST::BaseListener::_exitBashInCondition(std::shared_ptr<AST::ASTNode> node) {
	exitBashInCondition(std::dynamic_pointer_cast<AST::BashInCondition>(node));
}
void AST::BaseListener::_enterBashPipeline(std::shared_ptr<AST::ASTNode> node) {
	enterBashPipeline(std::dynamic_pointer_cast<AST::BashPipeline>(node));
}
void AST::BaseListener::_exitBashPipeline(std::shared_ptr<AST::ASTNode> node) {
	exitBashPipeline(std::dynamic_pointer_cast<AST::BashPipeline>(node));
}
void AST::BaseListener::_enterBashRedirection(std::shared_ptr<AST::ASTNode> node) {
	enterBashRedirection(std::dynamic_pointer_cast<AST::BashRedirection>(node));
}
void AST::BaseListener::_exitBashRedirection(std::shared_ptr<AST::ASTNode> node) {
	exitBashRedirection(std::dynamic_pointer_cast<AST::BashRedirection>(node));
}
void AST::BaseListener::_enterBashSelectStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashSelectStatement(std::dynamic_pointer_cast<AST::BashSelectStatement>(node));
}
void AST::BaseListener::_exitBashSelectStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashSelectStatement(std::dynamic_pointer_cast<AST::BashSelectStatement>(node));
}
void AST::BaseListener::_enterBashUntilStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashUntilStatement(std::dynamic_pointer_cast<AST::BashUntilStatement>(node));
}
void AST::BaseListener::_exitBashUntilStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashUntilStatement(std::dynamic_pointer_cast<AST::BashUntilStatement>(node));
}
void AST::BaseListener::_enterBashVariable(std::shared_ptr<AST::ASTNode> node) {
	enterBashVariable(std::dynamic_pointer_cast<AST::BashVariable>(node));
}
void AST::BaseListener::_exitBashVariable(std::shared_ptr<AST::ASTNode> node) {
	exitBashVariable(std::dynamic_pointer_cast<AST::BashVariable>(node));
}
void AST::BaseListener::_enterBashWhileOrUntilCondition(std::shared_ptr<AST::ASTNode> node) {
	enterBashWhileOrUntilCondition(std::dynamic_pointer_cast<AST::BashWhileOrUntilCondition>(node));
}
void AST::BaseListener::_exitBashWhileOrUntilCondition(std::shared_ptr<AST::ASTNode> node) {
	exitBashWhileOrUntilCondition(std::dynamic_pointer_cast<AST::BashWhileOrUntilCondition>(node));
}
void AST::BaseListener::_enterBashWhileStatement(std::shared_ptr<AST::ASTNode> node) {
	enterBashWhileStatement(std::dynamic_pointer_cast<AST::BashWhileStatement>(node));
}
void AST::BaseListener::_exitBashWhileStatement(std::shared_ptr<AST::ASTNode> node) {
	exitBashWhileStatement(std::dynamic_pointer_cast<AST::BashWhileStatement>(node));
}
void AST::BaseListener::_enterBlock(std::shared_ptr<AST::ASTNode> node) {
	enterBlock(std::dynamic_pointer_cast<AST::Block>(node));
}
void AST::BaseListener::_exitBlock(std::shared_ptr<AST::ASTNode> node) {
	exitBlock(std::dynamic_pointer_cast<AST::Block>(node));
}
void AST::BaseListener::_enterClassDefinition(std::shared_ptr<AST::ASTNode> node) {
	enterClassDefinition(std::dynamic_pointer_cast<AST::ClassDefinition>(node));
}
void AST::BaseListener::_exitClassDefinition(std::shared_ptr<AST::ASTNode> node) {
	exitClassDefinition(std::dynamic_pointer_cast<AST::ClassDefinition>(node));
}
void AST::BaseListener::_enterConstructorDefinition(std::shared_ptr<AST::ASTNode> node) {
	enterConstructorDefinition(std::dynamic_pointer_cast<AST::ConstructorDefinition>(node));
}
void AST::BaseListener::_exitConstructorDefinition(std::shared_ptr<AST::ASTNode> node) {
	exitConstructorDefinition(std::dynamic_pointer_cast<AST::ConstructorDefinition>(node));
}
void AST::BaseListener::_enterDatamemberDeclaration(std::shared_ptr<AST::ASTNode> node) {
	enterDatamemberDeclaration(std::dynamic_pointer_cast<AST::DatamemberDeclaration>(node));
}
void AST::BaseListener::_exitDatamemberDeclaration(std::shared_ptr<AST::ASTNode> node) {
	exitDatamemberDeclaration(std::dynamic_pointer_cast<AST::DatamemberDeclaration>(node));
}
void AST::BaseListener::_enterDeleteStatement(std::shared_ptr<AST::ASTNode> node) {
	enterDeleteStatement(std::dynamic_pointer_cast<AST::DeleteStatement>(node));
}
void AST::BaseListener::_exitDeleteStatement(std::shared_ptr<AST::ASTNode> node) {
	exitDeleteStatement(std::dynamic_pointer_cast<AST::DeleteStatement>(node));
}
void AST::BaseListener::_enterDestructorDefinition(std::shared_ptr<AST::ASTNode> node) {
	enterDestructorDefinition(std::dynamic_pointer_cast<AST::DestructorDefinition>(node));
}
void AST::BaseListener::_exitDestructorDefinition(std::shared_ptr<AST::ASTNode> node) {
	exitDestructorDefinition(std::dynamic_pointer_cast<AST::DestructorDefinition>(node));
}
void AST::BaseListener::_enterDoublequotedString(std::shared_ptr<AST::ASTNode> node) {
	enterDoublequotedString(std::dynamic_pointer_cast<AST::DoublequotedString>(node));
}
void AST::BaseListener::_exitDoublequotedString(std::shared_ptr<AST::ASTNode> node) {
	exitDoublequotedString(std::dynamic_pointer_cast<AST::DoublequotedString>(node));
}
void AST::BaseListener::_enterDynamicCast(std::shared_ptr<AST::ASTNode> node) {
	enterDynamicCast(std::dynamic_pointer_cast<AST::DynamicCast>(node));
}
void AST::BaseListener::_exitDynamicCast(std::shared_ptr<AST::ASTNode> node) {
	exitDynamicCast(std::dynamic_pointer_cast<AST::DynamicCast>(node));
}
void AST::BaseListener::_enterDynamicCastTarget(std::shared_ptr<AST::ASTNode> node) {
	enterDynamicCastTarget(std::dynamic_pointer_cast<AST::DynamicCastTarget>(node));
}
void AST::BaseListener::_exitDynamicCastTarget(std::shared_ptr<AST::ASTNode> node) {
	exitDynamicCastTarget(std::dynamic_pointer_cast<AST::DynamicCastTarget>(node));
}
void AST::BaseListener::_enterHeredocBody(std::shared_ptr<AST::ASTNode> node) {
	enterHeredocBody(std::dynamic_pointer_cast<AST::HeredocBody>(node));
}
void AST::BaseListener::_exitHeredocBody(std::shared_ptr<AST::ASTNode> node) {
	exitHeredocBody(std::dynamic_pointer_cast<AST::HeredocBody>(node));
}
void AST::BaseListener::_enterHereString(std::shared_ptr<AST::ASTNode> node) {
	enterHereString(std::dynamic_pointer_cast<AST::HereString>(node));
}
void AST::BaseListener::_exitHereString(std::shared_ptr<AST::ASTNode> node) {
	exitHereString(std::dynamic_pointer_cast<AST::HereString>(node));
}
void AST::BaseListener::_enterIncludeStatement(std::shared_ptr<AST::ASTNode> node) {
	enterIncludeStatement(std::dynamic_pointer_cast<AST::IncludeStatement>(node));
}
void AST::BaseListener::_exitIncludeStatement(std::shared_ptr<AST::ASTNode> node) {
	exitIncludeStatement(std::dynamic_pointer_cast<AST::IncludeStatement>(node));
}
void AST::BaseListener::_enterMethodDefinition(std::shared_ptr<AST::ASTNode> node) {
	enterMethodDefinition(std::dynamic_pointer_cast<AST::MethodDefinition>(node));
}
void AST::BaseListener::_exitMethodDefinition(std::shared_ptr<AST::ASTNode> node) {
	exitMethodDefinition(std::dynamic_pointer_cast<AST::MethodDefinition>(node));
}
void AST::BaseListener::_enterNewStatement(std::shared_ptr<AST::ASTNode> node) {
	enterNewStatement(std::dynamic_pointer_cast<AST::NewStatement>(node));
}
void AST::BaseListener::_exitNewStatement(std::shared_ptr<AST::ASTNode> node) {
	exitNewStatement(std::dynamic_pointer_cast<AST::NewStatement>(node));
}
void AST::BaseListener::_enterObjectAssignment(std::shared_ptr<AST::ASTNode> node) {
	enterObjectAssignment(std::dynamic_pointer_cast<AST::ObjectAssignment>(node));
}
void AST::BaseListener::_exitObjectAssignment(std::shared_ptr<AST::ASTNode> node) {
	exitObjectAssignment(std::dynamic_pointer_cast<AST::ObjectAssignment>(node));
}
void AST::BaseListener::_enterObjectInstantiation(std::shared_ptr<AST::ASTNode> node) {
	enterObjectInstantiation(std::dynamic_pointer_cast<AST::ObjectInstantiation>(node));
}
void AST::BaseListener::_exitObjectInstantiation(std::shared_ptr<AST::ASTNode> node) {
	exitObjectInstantiation(std::dynamic_pointer_cast<AST::ObjectInstantiation>(node));
}
void AST::BaseListener::_enterObjectReference(std::shared_ptr<AST::ASTNode> node) {
	enterObjectReference(std::dynamic_pointer_cast<AST::ObjectReference>(node));
}
void AST::BaseListener::_exitObjectReference(std::shared_ptr<AST::ASTNode> node) {
	exitObjectReference(std::dynamic_pointer_cast<AST::ObjectReference>(node));
}
void AST::BaseListener::_enterParameterExpansion(std::shared_ptr<AST::ASTNode> node) {
	enterParameterExpansion(std::dynamic_pointer_cast<AST::ParameterExpansion>(node));
}
void AST::BaseListener::_exitParameterExpansion(std::shared_ptr<AST::ASTNode> node) {
	exitParameterExpansion(std::dynamic_pointer_cast<AST::ParameterExpansion>(node));
}
void AST::BaseListener::_enterPointerDeclaration(std::shared_ptr<AST::ASTNode> node) {
	enterPointerDeclaration(std::dynamic_pointer_cast<AST::PointerDeclaration>(node));
}
void AST::BaseListener::_exitPointerDeclaration(std::shared_ptr<AST::ASTNode> node) {
	exitPointerDeclaration(std::dynamic_pointer_cast<AST::PointerDeclaration>(node));
}
void AST::BaseListener::_enterPrimitiveAssignment(std::shared_ptr<AST::ASTNode> node) {
	enterPrimitiveAssignment(std::dynamic_pointer_cast<AST::PrimitiveAssignment>(node));
}
void AST::BaseListener::_exitPrimitiveAssignment(std::shared_ptr<AST::ASTNode> node) {
	exitPrimitiveAssignment(std::dynamic_pointer_cast<AST::PrimitiveAssignment>(node));
}
void AST::BaseListener::_enterProcessSubstitution(std::shared_ptr<AST::ASTNode> node) {
	enterProcessSubstitution(std::dynamic_pointer_cast<AST::ProcessSubstitution>(node));
}
void AST::BaseListener::_exitProcessSubstitution(std::shared_ptr<AST::ASTNode> node) {
	exitProcessSubstitution(std::dynamic_pointer_cast<AST::ProcessSubstitution>(node));
}
void AST::BaseListener::_enterProgram(std::shared_ptr<AST::ASTNode> node) {
	enterProgram(std::dynamic_pointer_cast<AST::Program>(node));
}
void AST::BaseListener::_exitProgram(std::shared_ptr<AST::ASTNode> node) {
	exitProgram(std::dynamic_pointer_cast<AST::Program>(node));
}
void AST::BaseListener::_enterRawSubshell(std::shared_ptr<AST::ASTNode> node) {
	enterRawSubshell(std::dynamic_pointer_cast<AST::RawSubshell>(node));
}
void AST::BaseListener::_exitRawSubshell(std::shared_ptr<AST::ASTNode> node) {
	exitRawSubshell(std::dynamic_pointer_cast<AST::RawSubshell>(node));
}
void AST::BaseListener::_enterRawText(std::shared_ptr<AST::ASTNode> node) {
	enterRawText(std::dynamic_pointer_cast<AST::RawText>(node));
}
void AST::BaseListener::_exitRawText(std::shared_ptr<AST::ASTNode> node) {
	exitRawText(std::dynamic_pointer_cast<AST::RawText>(node));
}
void AST::BaseListener::_enterRvalue(std::shared_ptr<AST::ASTNode> node) {
	enterRvalue(std::dynamic_pointer_cast<AST::Rvalue>(node));
}
void AST::BaseListener::_exitRvalue(std::shared_ptr<AST::ASTNode> node) {
	exitRvalue(std::dynamic_pointer_cast<AST::Rvalue>(node));
}
void AST::BaseListener::_enterSubshellSubstitution(std::shared_ptr<AST::ASTNode> node) {
	enterSubshellSubstitution(std::dynamic_pointer_cast<AST::SubshellSubstitution>(node));
}
void AST::BaseListener::_exitSubshellSubstitution(std::shared_ptr<AST::ASTNode> node) {
	exitSubshellSubstitution(std::dynamic_pointer_cast<AST::SubshellSubstitution>(node));
}
void AST::BaseListener::_enterSupershell(std::shared_ptr<AST::ASTNode> node) {
	enterSupershell(std::dynamic_pointer_cast<AST::Supershell>(node));
}
void AST::BaseListener::_exitSupershell(std::shared_ptr<AST::ASTNode> node) {
	exitSupershell(std::dynamic_pointer_cast<AST::Supershell>(node));
}
void AST::BaseListener::_enterTypeofExpression(std::shared_ptr<AST::ASTNode> node) {
	enterTypeofExpression(std::dynamic_pointer_cast<AST::TypeofExpression>(node));
}
void AST::BaseListener::_exitTypeofExpression(std::shared_ptr<AST::ASTNode> node) {
	exitTypeofExpression(std::dynamic_pointer_cast<AST::TypeofExpression>(node));
}
void AST::BaseListener::_enterValueAssignment(std::shared_ptr<AST::ASTNode> node) {
	enterValueAssignment(std::dynamic_pointer_cast<AST::ValueAssignment>(node));
}
void AST::BaseListener::_exitValueAssignment(std::shared_ptr<AST::ASTNode> node) {
	exitValueAssignment(std::dynamic_pointer_cast<AST::ValueAssignment>(node));
}
