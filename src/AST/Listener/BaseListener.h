/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <memory>
#include <frozen/unordered_map.h>

#include "../ASTNode.h"
#include "../NodeTypes.h"
#include "../Nodes/Nodes.h"

namespace AST {

/**
 * @class BaseListener
 * @brief The base class for the Bash++ AST listener.
 * This class provides empty implementations for all enter and exit methods for each AST node type.
 * Each method can be overridden in derived classes to implement custom behavior when entering or exiting specific AST nodes.
 * The class also contains a non-virtual method `walk()` that dispatches to the appropriate enter and exit methods based on the node type.
 * 
 */
class BaseListener {
	private:
		/**
		 * @var enterExitMap
		 * @brief A fully constexpr map of AST node types to their corresponding enter and exit functions.
		 * 
		 */
		frozen::unordered_map<
			AST::NodeType,
			std::pair<
				std::function<void(std::shared_ptr<AST::ASTNode>)>,
				std::function<void(std::shared_ptr<AST::ASTNode>)>
			>,
			55
		> enterExitMap = {
			{ AST::NodeType::ArrayIndex, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterArrayIndex(std::dynamic_pointer_cast<AST::ArrayIndex>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitArrayIndex(std::dynamic_pointer_cast<AST::ArrayIndex>(node)); }
			} },
			{ AST::NodeType::BashArithmeticForCondition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashArithmeticForCondition(std::dynamic_pointer_cast<AST::BashArithmeticForCondition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashArithmeticForCondition(std::dynamic_pointer_cast<AST::BashArithmeticForCondition>(node)); }
			} },
			{ AST::NodeType::BashArithmeticForStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashArithmeticForStatement(std::dynamic_pointer_cast<AST::BashArithmeticForStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashArithmeticForStatement(std::dynamic_pointer_cast<AST::BashArithmeticForStatement>(node)); }
			} },
			{ AST::NodeType::BashArithmeticStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashArithmeticStatement(std::dynamic_pointer_cast<AST::BashArithmeticStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashArithmeticStatement(std::dynamic_pointer_cast<AST::BashArithmeticStatement>(node)); }
			} },
			{
				AST::NodeType::BashArithmeticSubstitution, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashArithmeticSubstitution(std::dynamic_pointer_cast<AST::BashArithmeticSubstitution>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashArithmeticSubstitution(std::dynamic_pointer_cast<AST::BashArithmeticSubstitution>(node)); }
			} },
			{ AST::NodeType::BashCaseInput, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCaseInput(std::dynamic_pointer_cast<AST::BashCaseInput>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCaseInput(std::dynamic_pointer_cast<AST::BashCaseInput>(node)); }
			} },
			{ AST::NodeType::BashCasePatternAction, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCasePatternAction(std::dynamic_pointer_cast<AST::BashCasePatternAction>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCasePatternAction(std::dynamic_pointer_cast<AST::BashCasePatternAction>(node)); }
			} },
			{ AST::NodeType::BashCasePattern, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCasePattern(std::dynamic_pointer_cast<AST::BashCasePattern>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCasePattern(std::dynamic_pointer_cast<AST::BashCasePattern>(node)); }
			} },
			{ AST::NodeType::BashCasePatternHeader, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCasePatternHeader(std::dynamic_pointer_cast<AST::BashCasePatternHeader>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCasePatternHeader(std::dynamic_pointer_cast<AST::BashCasePatternHeader>(node)); }
			} },
			{ AST::NodeType::BashCaseStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCaseStatement(std::dynamic_pointer_cast<AST::BashCaseStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCaseStatement(std::dynamic_pointer_cast<AST::BashCaseStatement>(node)); }
			} },
			{ AST::NodeType::BashCommand, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCommand(std::dynamic_pointer_cast<AST::BashCommand>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCommand(std::dynamic_pointer_cast<AST::BashCommand>(node)); }
			} },
			{ AST::NodeType::BashCommandSequence, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashCommandSequence(std::dynamic_pointer_cast<AST::BashCommandSequence>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashCommandSequence(std::dynamic_pointer_cast<AST::BashCommandSequence>(node)); }
			} },
			{ AST::NodeType::BashForStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashForStatement(std::dynamic_pointer_cast<AST::BashForStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashForStatement(std::dynamic_pointer_cast<AST::BashForStatement>(node)); }
			} },
			{ AST::NodeType::BashIfCondition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashIfCondition(std::dynamic_pointer_cast<AST::BashIfCondition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashIfCondition(std::dynamic_pointer_cast<AST::BashIfCondition>(node)); }
			} },
			{ AST::NodeType::BashIfElseBranch, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashIfElseBranch(std::dynamic_pointer_cast<AST::BashIfElseBranch>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashIfElseBranch(std::dynamic_pointer_cast<AST::BashIfElseBranch>(node)); }
			} },
			{ AST::NodeType::BashIfRootBranch, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashIfRootBranch(std::dynamic_pointer_cast<AST::BashIfRootBranch>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashIfRootBranch(std::dynamic_pointer_cast<AST::BashIfRootBranch>(node)); }
			} },
			{ AST::NodeType::BashIfStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashIfStatement(std::dynamic_pointer_cast<AST::BashIfStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashIfStatement(std::dynamic_pointer_cast<AST::BashIfStatement>(node)); }
			} },
			{ AST::NodeType::BashInCondition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashInCondition(std::dynamic_pointer_cast<AST::BashInCondition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashInCondition(std::dynamic_pointer_cast<AST::BashInCondition>(node)); }
			} },
			{ AST::NodeType::BashPipeline, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashPipeline(std::dynamic_pointer_cast<AST::BashPipeline>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashPipeline(std::dynamic_pointer_cast<AST::BashPipeline>(node)); }
			} },
			{ AST::NodeType::BashRedirection, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashRedirection(std::dynamic_pointer_cast<AST::BashRedirection>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashRedirection(std::dynamic_pointer_cast<AST::BashRedirection>(node)); }
			} },
			{ AST::NodeType::BashSelectStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashSelectStatement(std::dynamic_pointer_cast<AST::BashSelectStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashSelectStatement(std::dynamic_pointer_cast<AST::BashSelectStatement>(node)); }
			} },
			{ AST::NodeType::BashUntilStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashUntilStatement(std::dynamic_pointer_cast<AST::BashUntilStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashUntilStatement(std::dynamic_pointer_cast<AST::BashUntilStatement>(node)); }
			} },
			{ AST::NodeType::BashVariable, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashVariable(std::dynamic_pointer_cast<AST::BashVariable>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashVariable(std::dynamic_pointer_cast<AST::BashVariable>(node)); }
			} },
			{ AST::NodeType::BashWhileOrUntilCondition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashWhileOrUntilCondition(std::dynamic_pointer_cast<AST::BashWhileOrUntilCondition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashWhileOrUntilCondition(std::dynamic_pointer_cast<AST::BashWhileOrUntilCondition>(node)); }
			} },
			{ AST::NodeType::BashWhileStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashWhileStatement(std::dynamic_pointer_cast<AST::BashWhileStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashWhileStatement(std::dynamic_pointer_cast<AST::BashWhileStatement>(node)); }
			} },
			{
				AST::NodeType::BashFunction, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBashFunction(std::dynamic_pointer_cast<AST::BashFunction>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBashFunction(std::dynamic_pointer_cast<AST::BashFunction>(node)); }
			} },
			{ AST::NodeType::Block, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterBlock(std::dynamic_pointer_cast<AST::Block>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitBlock(std::dynamic_pointer_cast<AST::Block>(node)); }
			} },
			{ AST::NodeType::ClassDefinition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterClassDefinition(std::dynamic_pointer_cast<AST::ClassDefinition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitClassDefinition(std::dynamic_pointer_cast<AST::ClassDefinition>(node)); }
			} },
			{ AST::NodeType::ConstructorDefinition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterConstructorDefinition(std::dynamic_pointer_cast<AST::ConstructorDefinition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitConstructorDefinition(std::dynamic_pointer_cast<AST::ConstructorDefinition>(node)); }
			} },
			{ AST::NodeType::DatamemberDeclaration, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterDatamemberDeclaration(std::dynamic_pointer_cast<AST::DatamemberDeclaration>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitDatamemberDeclaration(std::dynamic_pointer_cast<AST::DatamemberDeclaration>(node)); }
			} },
			{ AST::NodeType::DeleteStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterDeleteStatement(std::dynamic_pointer_cast<AST::DeleteStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitDeleteStatement(std::dynamic_pointer_cast<AST::DeleteStatement>(node)); }
			} },
			{ AST::NodeType::DestructorDefinition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterDestructorDefinition(std::dynamic_pointer_cast<AST::DestructorDefinition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitDestructorDefinition(std::dynamic_pointer_cast<AST::DestructorDefinition>(node)); }
			} },
			{ AST::NodeType::DoublequotedString, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterDoublequotedString(std::dynamic_pointer_cast<AST::DoublequotedString>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitDoublequotedString(std::dynamic_pointer_cast<AST::DoublequotedString>(node)); }
			} },
			{ AST::NodeType::DynamicCast, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterDynamicCast(std::dynamic_pointer_cast<AST::DynamicCast>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitDynamicCast(std::dynamic_pointer_cast<AST::DynamicCast>(node)); }
			} },
			{ AST::NodeType::DynamicCastTarget, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterDynamicCastTarget(std::dynamic_pointer_cast<AST::DynamicCastTarget>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitDynamicCastTarget(std::dynamic_pointer_cast<AST::DynamicCastTarget>(node)); }
			} },
			{ AST::NodeType::HeredocBody, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterHeredocBody(std::dynamic_pointer_cast<AST::HeredocBody>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitHeredocBody(std::dynamic_pointer_cast<AST::HeredocBody>(node)); }
			} },
			{ AST::NodeType::HereString, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterHereString(std::dynamic_pointer_cast<AST::HereString>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitHereString(std::dynamic_pointer_cast<AST::HereString>(node)); }
			} },
			{ AST::NodeType::IncludeStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterIncludeStatement(std::dynamic_pointer_cast<AST::IncludeStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitIncludeStatement(std::dynamic_pointer_cast<AST::IncludeStatement>(node)); }
			} },
			{ AST::NodeType::MethodDefinition, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterMethodDefinition(std::dynamic_pointer_cast<AST::MethodDefinition>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitMethodDefinition(std::dynamic_pointer_cast<AST::MethodDefinition>(node)); }
			} },
			{ AST::NodeType::NewStatement, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterNewStatement(std::dynamic_pointer_cast<AST::NewStatement>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitNewStatement(std::dynamic_pointer_cast<AST::NewStatement>(node)); }
			} },
			{ AST::NodeType::ObjectAssignment, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterObjectAssignment(std::dynamic_pointer_cast<AST::ObjectAssignment>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitObjectAssignment(std::dynamic_pointer_cast<AST::ObjectAssignment>(node)); }
			} },
			{ AST::NodeType::ObjectInstantiation, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterObjectInstantiation(std::dynamic_pointer_cast<AST::ObjectInstantiation>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitObjectInstantiation(std::dynamic_pointer_cast<AST::ObjectInstantiation>(node)); }
			} },
			{ AST::NodeType::ObjectReference, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterObjectReference(std::dynamic_pointer_cast<AST::ObjectReference>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitObjectReference(std::dynamic_pointer_cast<AST::ObjectReference>(node)); }
			} },
			{ AST::NodeType::ParameterExpansion, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterParameterExpansion(std::dynamic_pointer_cast<AST::ParameterExpansion>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitParameterExpansion(std::dynamic_pointer_cast<AST::ParameterExpansion>(node)); }
			} },
			{ AST::NodeType::PointerDeclaration, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterPointerDeclaration(std::dynamic_pointer_cast<AST::PointerDeclaration>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitPointerDeclaration(std::dynamic_pointer_cast<AST::PointerDeclaration>(node)); }
			} },
			{ AST::NodeType::PrimitiveAssignment, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterPrimitiveAssignment(std::dynamic_pointer_cast<AST::PrimitiveAssignment>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitPrimitiveAssignment(std::dynamic_pointer_cast<AST::PrimitiveAssignment>(node)); }
			} },
			{ AST::NodeType::ProcessSubstitution, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterProcessSubstitution(std::dynamic_pointer_cast<AST::ProcessSubstitution>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitProcessSubstitution(std::dynamic_pointer_cast<AST::ProcessSubstitution>(node)); }
			} },
			{ AST::NodeType::Program, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterProgram(std::dynamic_pointer_cast<AST::Program>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitProgram(std::dynamic_pointer_cast<AST::Program>(node)); }
			} },
			{ AST::NodeType::RawSubshell, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterRawSubshell(std::dynamic_pointer_cast<AST::RawSubshell>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitRawSubshell(std::dynamic_pointer_cast<AST::RawSubshell>(node)); }
			} },
			{ AST::NodeType::RawText, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterRawText(std::dynamic_pointer_cast<AST::RawText>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitRawText(std::dynamic_pointer_cast<AST::RawText>(node)); }
			} },
			{ AST::NodeType::Rvalue, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterRvalue(std::dynamic_pointer_cast<AST::Rvalue>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitRvalue(std::dynamic_pointer_cast<AST::Rvalue>(node)); }
			} },
			{ AST::NodeType::SubshellSubstitution, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterSubshellSubstitution(std::dynamic_pointer_cast<AST::SubshellSubstitution>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitSubshellSubstitution(std::dynamic_pointer_cast<AST::SubshellSubstitution>(node)); }
			} },
			{ AST::NodeType::Supershell, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterSupershell(std::dynamic_pointer_cast<AST::Supershell>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitSupershell(std::dynamic_pointer_cast<AST::Supershell>(node)); }
			} },
			{ AST::NodeType::TypeofExpression, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterTypeofExpression(std::dynamic_pointer_cast<AST::TypeofExpression>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitTypeofExpression(std::dynamic_pointer_cast<AST::TypeofExpression>(node)); }
			} },
			{ AST::NodeType::ValueAssignment, { 
				[this](std::shared_ptr<AST::ASTNode> node) { enterValueAssignment(std::dynamic_pointer_cast<AST::ValueAssignment>(node)); },
				[this](std::shared_ptr<AST::ASTNode> node) { exitValueAssignment(std::dynamic_pointer_cast<AST::ValueAssignment>(node)); }
			} }
		};
	public:
		virtual ~BaseListener() = default;

		void walk(std::shared_ptr<AST::ASTNode> node);

		virtual void enterProgram([[maybe_unused]] std::shared_ptr<AST::Program> node) {}
		virtual void exitProgram([[maybe_unused]] std::shared_ptr<AST::Program> node) {}
		virtual void enterArrayIndex([[maybe_unused]] std::shared_ptr<AST::ArrayIndex> node) {}
		virtual void exitArrayIndex([[maybe_unused]] std::shared_ptr<AST::ArrayIndex> node) {}
		virtual void enterBashArithmeticForCondition([[maybe_unused]] std::shared_ptr<AST::BashArithmeticForCondition> node) {}
		virtual void exitBashArithmeticForCondition([[maybe_unused]] std::shared_ptr<AST::BashArithmeticForCondition> node) {}
		virtual void enterBashArithmeticForStatement([[maybe_unused]] std::shared_ptr<AST::BashArithmeticForStatement> node) {}
		virtual void exitBashArithmeticForStatement([[maybe_unused]] std::shared_ptr<AST::BashArithmeticForStatement> node) {}
		virtual void enterBashArithmeticStatement([[maybe_unused]] std::shared_ptr<AST::BashArithmeticStatement> node) {}
		virtual void exitBashArithmeticStatement([[maybe_unused]] std::shared_ptr<AST::BashArithmeticStatement> node) {}
		virtual void enterBashArithmeticSubstitution([[maybe_unused]] std::shared_ptr<AST::BashArithmeticSubstitution> node) {}
		virtual void exitBashArithmeticSubstitution([[maybe_unused]] std::shared_ptr<AST::BashArithmeticSubstitution> node) {}
		virtual void enterBashCaseInput([[maybe_unused]] std::shared_ptr<AST::BashCaseInput> node) {}
		virtual void exitBashCaseInput([[maybe_unused]] std::shared_ptr<AST::BashCaseInput> node) {}
		virtual void enterBashCasePatternAction([[maybe_unused]] std::shared_ptr<AST::BashCasePatternAction> node) {}
		virtual void exitBashCasePatternAction([[maybe_unused]] std::shared_ptr<AST::BashCasePatternAction> node) {}
		virtual void enterBashCasePattern([[maybe_unused]] std::shared_ptr<AST::BashCasePattern> node) {}
		virtual void exitBashCasePattern([[maybe_unused]] std::shared_ptr<AST::BashCasePattern> node) {}
		virtual void enterBashCasePatternHeader([[maybe_unused]] std::shared_ptr<AST::BashCasePatternHeader> node) {}
		virtual void exitBashCasePatternHeader([[maybe_unused]] std::shared_ptr<AST::BashCasePatternHeader> node) {}
		virtual void enterBashCaseStatement([[maybe_unused]] std::shared_ptr<AST::BashCaseStatement> node) {}
		virtual void exitBashCaseStatement([[maybe_unused]] std::shared_ptr<AST::BashCaseStatement> node) {}
		virtual void enterBashCommand([[maybe_unused]] std::shared_ptr<AST::BashCommand> node) {}
		virtual void exitBashCommand([[maybe_unused]] std::shared_ptr<AST::BashCommand> node) {}
		virtual void enterBashCommandSequence([[maybe_unused]] std::shared_ptr<AST::BashCommandSequence> node) {}
		virtual void exitBashCommandSequence([[maybe_unused]] std::shared_ptr<AST::BashCommandSequence> node) {}
		virtual void enterBashForStatement([[maybe_unused]] std::shared_ptr<AST::BashForStatement> node) {}
		virtual void exitBashForStatement([[maybe_unused]] std::shared_ptr<AST::BashForStatement> node) {}
		virtual void enterBashIfCondition([[maybe_unused]] std::shared_ptr<AST::BashIfCondition> node) {}
		virtual void exitBashIfCondition([[maybe_unused]] std::shared_ptr<AST::BashIfCondition> node) {}
		virtual void enterBashIfElseBranch([[maybe_unused]] std::shared_ptr<AST::BashIfElseBranch> node) {}
		virtual void exitBashIfElseBranch([[maybe_unused]] std::shared_ptr<AST::BashIfElseBranch> node) {}
		virtual void enterBashIfRootBranch([[maybe_unused]] std::shared_ptr<AST::BashIfRootBranch> node) {}
		virtual void exitBashIfRootBranch([[maybe_unused]] std::shared_ptr<AST::BashIfRootBranch> node) {}
		virtual void enterBashIfStatement([[maybe_unused]] std::shared_ptr<AST::BashIfStatement> node) {}
		virtual void exitBashIfStatement([[maybe_unused]] std::shared_ptr<AST::BashIfStatement> node) {}
		virtual void enterBashInCondition([[maybe_unused]] std::shared_ptr<AST::BashInCondition> node) {}
		virtual void exitBashInCondition([[maybe_unused]] std::shared_ptr<AST::BashInCondition> node) {}
		virtual void enterBashPipeline([[maybe_unused]] std::shared_ptr<AST::BashPipeline> node) {}
		virtual void exitBashPipeline([[maybe_unused]] std::shared_ptr<AST::BashPipeline> node) {}
		virtual void enterBashRedirection([[maybe_unused]] std::shared_ptr<AST::BashRedirection> node) {}
		virtual void exitBashRedirection([[maybe_unused]] std::shared_ptr<AST::BashRedirection> node) {}
		virtual void enterBashSelectStatement([[maybe_unused]] std::shared_ptr<AST::BashSelectStatement> node) {}
		virtual void exitBashSelectStatement([[maybe_unused]] std::shared_ptr<AST::BashSelectStatement> node) {}
		virtual void enterBashUntilStatement([[maybe_unused]] std::shared_ptr<AST::BashUntilStatement> node) {}
		virtual void exitBashUntilStatement([[maybe_unused]] std::shared_ptr<AST::BashUntilStatement> node) {}
		virtual void enterBashVariable([[maybe_unused]] std::shared_ptr<AST::BashVariable> node) {}
		virtual void exitBashVariable([[maybe_unused]] std::shared_ptr<AST::BashVariable> node) {}
		virtual void enterBashWhileOrUntilCondition([[maybe_unused]] std::shared_ptr<AST::BashWhileOrUntilCondition> node) {}
		virtual void exitBashWhileOrUntilCondition([[maybe_unused]] std::shared_ptr<AST::BashWhileOrUntilCondition> node) {}
		virtual void enterBashWhileStatement([[maybe_unused]] std::shared_ptr<AST::BashWhileStatement> node) {}
		virtual void exitBashWhileStatement([[maybe_unused]] std::shared_ptr<AST::BashWhileStatement> node) {}
		virtual void enterBashFunction([[maybe_unused]] std::shared_ptr<AST::BashFunction> node) {}
		virtual void exitBashFunction([[maybe_unused]] std::shared_ptr<AST::BashFunction> node) {}
		virtual void enterBlock([[maybe_unused]] std::shared_ptr<AST::Block> node) {}
		virtual void exitBlock([[maybe_unused]] std::shared_ptr<AST::Block> node) {}
		virtual void enterClassDefinition([[maybe_unused]] std::shared_ptr<AST::ClassDefinition> node) {}
		virtual void exitClassDefinition([[maybe_unused]] std::shared_ptr<AST::ClassDefinition> node) {}
		virtual void enterConstructorDefinition([[maybe_unused]] std::shared_ptr<AST::ConstructorDefinition> node) {}
		virtual void exitConstructorDefinition([[maybe_unused]] std::shared_ptr<AST::ConstructorDefinition> node) {}
		virtual void enterDatamemberDeclaration([[maybe_unused]] std::shared_ptr<AST::DatamemberDeclaration> node) {}
		virtual void exitDatamemberDeclaration([[maybe_unused]] std::shared_ptr<AST::DatamemberDeclaration> node) {}
		virtual void enterDeleteStatement([[maybe_unused]] std::shared_ptr<AST::DeleteStatement> node) {}
		virtual void exitDeleteStatement([[maybe_unused]] std::shared_ptr<AST::DeleteStatement> node) {}
		virtual void enterDestructorDefinition([[maybe_unused]] std::shared_ptr<AST::DestructorDefinition> node) {}
		virtual void exitDestructorDefinition([[maybe_unused]] std::shared_ptr<AST::DestructorDefinition> node) {}
		virtual void enterDoublequotedString([[maybe_unused]] std::shared_ptr<AST::DoublequotedString> node) {}
		virtual void exitDoublequotedString([[maybe_unused]] std::shared_ptr<AST::DoublequotedString> node) {}
		virtual void enterDynamicCast([[maybe_unused]] std::shared_ptr<AST::DynamicCast> node) {}
		virtual void exitDynamicCast([[maybe_unused]] std::shared_ptr<AST::DynamicCast> node) {}
		virtual void enterDynamicCastTarget([[maybe_unused]] std::shared_ptr<AST::DynamicCastTarget> node) {}
		virtual void exitDynamicCastTarget([[maybe_unused]] std::shared_ptr<AST::DynamicCastTarget> node) {}
		virtual void enterHeredocBody([[maybe_unused]] std::shared_ptr<AST::HeredocBody> node) {}
		virtual void exitHeredocBody([[maybe_unused]] std::shared_ptr<AST::HeredocBody> node) {}
		virtual void enterHereString([[maybe_unused]] std::shared_ptr<AST::HereString> node) {}
		virtual void exitHereString([[maybe_unused]] std::shared_ptr<AST::HereString> node) {}
		virtual void enterIncludeStatement([[maybe_unused]] std::shared_ptr<AST::IncludeStatement> node) {}
		virtual void exitIncludeStatement([[maybe_unused]] std::shared_ptr<AST::IncludeStatement> node) {}
		virtual void enterMethodDefinition([[maybe_unused]] std::shared_ptr<AST::MethodDefinition> node) {}
		virtual void exitMethodDefinition([[maybe_unused]] std::shared_ptr<AST::MethodDefinition> node) {}
		virtual void enterNewStatement([[maybe_unused]] std::shared_ptr<AST::NewStatement> node) {}
		virtual void exitNewStatement([[maybe_unused]] std::shared_ptr<AST::NewStatement> node) {}
		virtual void enterObjectAssignment([[maybe_unused]] std::shared_ptr<AST::ObjectAssignment> node) {}
		virtual void exitObjectAssignment([[maybe_unused]] std::shared_ptr<AST::ObjectAssignment> node) {}
		virtual void enterObjectInstantiation([[maybe_unused]] std::shared_ptr<AST::ObjectInstantiation> node) {}
		virtual void exitObjectInstantiation([[maybe_unused]] std::shared_ptr<AST::ObjectInstantiation> node) {}
		virtual void enterObjectReference([[maybe_unused]] std::shared_ptr<AST::ObjectReference> node) {}
		virtual void exitObjectReference([[maybe_unused]] std::shared_ptr<AST::ObjectReference> node) {}
		virtual void enterParameterExpansion([[maybe_unused]] std::shared_ptr<AST::ParameterExpansion> node) {}
		virtual void exitParameterExpansion([[maybe_unused]] std::shared_ptr<AST::ParameterExpansion> node) {}
		virtual void enterPointerDeclaration([[maybe_unused]] std::shared_ptr<AST::PointerDeclaration> node) {}
		virtual void exitPointerDeclaration([[maybe_unused]] std::shared_ptr<AST::PointerDeclaration> node) {}
		virtual void enterPrimitiveAssignment([[maybe_unused]] std::shared_ptr<AST::PrimitiveAssignment> node) {}
		virtual void exitPrimitiveAssignment([[maybe_unused]] std::shared_ptr<AST::PrimitiveAssignment> node) {}
		virtual void enterProcessSubstitution([[maybe_unused]] std::shared_ptr<AST::ProcessSubstitution> node) {}
		virtual void exitProcessSubstitution([[maybe_unused]] std::shared_ptr<AST::ProcessSubstitution> node) {}
		virtual void enterRawSubshell([[maybe_unused]] std::shared_ptr<AST::RawSubshell> node) {}
		virtual void exitRawSubshell([[maybe_unused]] std::shared_ptr<AST::RawSubshell> node) {}
		virtual void enterRawText([[maybe_unused]] std::shared_ptr<AST::RawText> node) {}
		virtual void exitRawText([[maybe_unused]] std::shared_ptr<AST::RawText> node) {}
		virtual void enterRvalue([[maybe_unused]] std::shared_ptr<AST::Rvalue> node) {}
		virtual void exitRvalue([[maybe_unused]] std::shared_ptr<AST::Rvalue> node) {}
		virtual void enterSubshellSubstitution([[maybe_unused]] std::shared_ptr<AST::SubshellSubstitution> node) {}
		virtual void exitSubshellSubstitution([[maybe_unused]] std::shared_ptr<AST::SubshellSubstitution> node) {}
		virtual void enterSupershell([[maybe_unused]] std::shared_ptr<AST::Supershell> node) {}
		virtual void exitSupershell([[maybe_unused]] std::shared_ptr<AST::Supershell> node) {}
		virtual void enterTypeofExpression([[maybe_unused]] std::shared_ptr<AST::TypeofExpression> node) {}
		virtual void exitTypeofExpression([[maybe_unused]] std::shared_ptr<AST::TypeofExpression> node) {}
		virtual void enterValueAssignment([[maybe_unused]] std::shared_ptr<AST::ValueAssignment> node) {}
		virtual void exitValueAssignment([[maybe_unused]] std::shared_ptr<AST::ValueAssignment> node) {}
};

} // namespace AST
