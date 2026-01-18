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
		void _enterArrayIndex(std::shared_ptr<AST::ASTNode> node);
		void _exitArrayIndex(std::shared_ptr<AST::ASTNode> node);
		void _enterBashArithmeticForCondition(std::shared_ptr<AST::ASTNode> node);
		void _exitBashArithmeticForCondition(std::shared_ptr<AST::ASTNode> node);
		void _enterBashArithmeticForStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashArithmeticForStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashArithmeticStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashArithmeticStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashArithmeticSubstitution(std::shared_ptr<AST::ASTNode> node);
		void _exitBashArithmeticSubstitution(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCaseInput(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCaseInput(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCasePatternAction(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCasePatternAction(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCasePattern(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCasePattern(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCasePatternHeader(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCasePatternHeader(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCaseStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCaseStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCommand(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCommand(std::shared_ptr<AST::ASTNode> node);
		void _enterBashCommandSequence(std::shared_ptr<AST::ASTNode> node);
		void _exitBashCommandSequence(std::shared_ptr<AST::ASTNode> node);
		void _enterBashForStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashForStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashFunction(std::shared_ptr<AST::ASTNode> node);
		void _exitBashFunction(std::shared_ptr<AST::ASTNode> node);
		void _enterBashIfCondition(std::shared_ptr<AST::ASTNode> node);
		void _exitBashIfCondition(std::shared_ptr<AST::ASTNode> node);
		void _enterBashIfElseBranch(std::shared_ptr<AST::ASTNode> node);
		void _exitBashIfElseBranch(std::shared_ptr<AST::ASTNode> node);
		void _enterBashIfRootBranch(std::shared_ptr<AST::ASTNode> node);
		void _exitBashIfRootBranch(std::shared_ptr<AST::ASTNode> node);
		void _enterBashIfStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashIfStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashInCondition(std::shared_ptr<AST::ASTNode> node);
		void _exitBashInCondition(std::shared_ptr<AST::ASTNode> node);
		void _enterBashPipeline(std::shared_ptr<AST::ASTNode> node);
		void _exitBashPipeline(std::shared_ptr<AST::ASTNode> node);
		void _enterBashRedirection(std::shared_ptr<AST::ASTNode> node);
		void _exitBashRedirection(std::shared_ptr<AST::ASTNode> node);
		void _enterBashSelectStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashSelectStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashUntilStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashUntilStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBashVariable(std::shared_ptr<AST::ASTNode> node);
		void _exitBashVariable(std::shared_ptr<AST::ASTNode> node);
		void _enterBashWhileOrUntilCondition(std::shared_ptr<AST::ASTNode> node);
		void _exitBashWhileOrUntilCondition(std::shared_ptr<AST::ASTNode> node);
		void _enterBashWhileStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitBashWhileStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterBlock(std::shared_ptr<AST::ASTNode> node);
		void _exitBlock(std::shared_ptr<AST::ASTNode> node);
		void _enterClassDefinition(std::shared_ptr<AST::ASTNode> node);
		void _exitClassDefinition(std::shared_ptr<AST::ASTNode> node);
		void _enterConstructorDefinition(std::shared_ptr<AST::ASTNode> node);
		void _exitConstructorDefinition(std::shared_ptr<AST::ASTNode> node);
		void _enterDatamemberDeclaration(std::shared_ptr<AST::ASTNode> node);
		void _exitDatamemberDeclaration(std::shared_ptr<AST::ASTNode> node);
		void _enterDeleteStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitDeleteStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterDestructorDefinition(std::shared_ptr<AST::ASTNode> node);
		void _exitDestructorDefinition(std::shared_ptr<AST::ASTNode> node);
		void _enterDoublequotedString(std::shared_ptr<AST::ASTNode> node);
		void _exitDoublequotedString(std::shared_ptr<AST::ASTNode> node);
		void _enterDynamicCast(std::shared_ptr<AST::ASTNode> node);
		void _exitDynamicCast(std::shared_ptr<AST::ASTNode> node);
		void _enterDynamicCastTarget(std::shared_ptr<AST::ASTNode> node);
		void _exitDynamicCastTarget(std::shared_ptr<AST::ASTNode> node);
		void _enterHeredocBody(std::shared_ptr<AST::ASTNode> node);
		void _exitHeredocBody(std::shared_ptr<AST::ASTNode> node);
		void _enterHereString(std::shared_ptr<AST::ASTNode> node);
		void _exitHereString(std::shared_ptr<AST::ASTNode> node);
		void _enterIncludeStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitIncludeStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterMethodDefinition(std::shared_ptr<AST::ASTNode> node);
		void _exitMethodDefinition(std::shared_ptr<AST::ASTNode> node);
		void _enterNewStatement(std::shared_ptr<AST::ASTNode> node);
		void _exitNewStatement(std::shared_ptr<AST::ASTNode> node);
		void _enterObjectAssignment(std::shared_ptr<AST::ASTNode> node);
		void _exitObjectAssignment(std::shared_ptr<AST::ASTNode> node);
		void _enterObjectInstantiation(std::shared_ptr<AST::ASTNode> node);
		void _exitObjectInstantiation(std::shared_ptr<AST::ASTNode> node);
		void _enterObjectReference(std::shared_ptr<AST::ASTNode> node);
		void _exitObjectReference(std::shared_ptr<AST::ASTNode> node);
		void _enterParameterExpansion(std::shared_ptr<AST::ASTNode> node);
		void _exitParameterExpansion(std::shared_ptr<AST::ASTNode> node);
		void _enterPointerDeclaration(std::shared_ptr<AST::ASTNode> node);
		void _exitPointerDeclaration(std::shared_ptr<AST::ASTNode> node);
		void _enterPrimitiveAssignment(std::shared_ptr<AST::ASTNode> node);
		void _exitPrimitiveAssignment(std::shared_ptr<AST::ASTNode> node);
		void _enterProcessSubstitution(std::shared_ptr<AST::ASTNode> node);
		void _exitProcessSubstitution(std::shared_ptr<AST::ASTNode> node);
		void _enterProgram(std::shared_ptr<AST::ASTNode> node);
		void _exitProgram(std::shared_ptr<AST::ASTNode> node);
		void _enterRawSubshell(std::shared_ptr<AST::ASTNode> node);
		void _exitRawSubshell(std::shared_ptr<AST::ASTNode> node);
		void _enterRawText(std::shared_ptr<AST::ASTNode> node);
		void _exitRawText(std::shared_ptr<AST::ASTNode> node);
		void _enterRvalue(std::shared_ptr<AST::ASTNode> node);
		void _exitRvalue(std::shared_ptr<AST::ASTNode> node);
		void _enterSubshellSubstitution(std::shared_ptr<AST::ASTNode> node);
		void _exitSubshellSubstitution(std::shared_ptr<AST::ASTNode> node);
		void _enterSupershell(std::shared_ptr<AST::ASTNode> node);
		void _exitSupershell(std::shared_ptr<AST::ASTNode> node);
		void _enterTypeofExpression(std::shared_ptr<AST::ASTNode> node);
		void _exitTypeofExpression(std::shared_ptr<AST::ASTNode> node);
		void _enterValueAssignment(std::shared_ptr<AST::ASTNode> node);
		void _exitValueAssignment(std::shared_ptr<AST::ASTNode> node);

		/**
		 * @brief A fully constexpr map of AST node types to their corresponding enter and exit functions.
		 */
		static constexpr frozen::unordered_map<
			AST::NodeType,
			std::pair<
				void (BaseListener::*)(std::shared_ptr<AST::ASTNode>),
				void (BaseListener::*)(std::shared_ptr<AST::ASTNode>)
			>,
			55
		> enterExitMap = {
			{ AST::NodeType::ArrayIndex, { &BaseListener::_enterArrayIndex, &BaseListener::_exitArrayIndex } },
		{ AST::NodeType::BashArithmeticForCondition, { &BaseListener::_enterBashArithmeticForCondition, &BaseListener::_exitBashArithmeticForCondition } },
		{ AST::NodeType::BashArithmeticForStatement, { &BaseListener::_enterBashArithmeticForStatement, &BaseListener::_exitBashArithmeticForStatement } },
		{ AST::NodeType::BashArithmeticStatement, { &BaseListener::_enterBashArithmeticStatement, &BaseListener::_exitBashArithmeticStatement } },
		{ AST::NodeType::BashArithmeticSubstitution, { &BaseListener::_enterBashArithmeticSubstitution, &BaseListener::_exitBashArithmeticSubstitution } },
		{ AST::NodeType::BashCaseInput, { &BaseListener::_enterBashCaseInput, &BaseListener::_exitBashCaseInput } },
		{ AST::NodeType::BashCasePatternAction, { &BaseListener::_enterBashCasePatternAction, &BaseListener::_exitBashCasePatternAction } },
		{ AST::NodeType::BashCasePattern, { &BaseListener::_enterBashCasePattern, &BaseListener::_exitBashCasePattern } },
		{ AST::NodeType::BashCasePatternHeader, { &BaseListener::_enterBashCasePatternHeader, &BaseListener::_exitBashCasePatternHeader } },
		{ AST::NodeType::BashCaseStatement, { &BaseListener::_enterBashCaseStatement, &BaseListener::_exitBashCaseStatement } },
		{ AST::NodeType::BashCommand, { &BaseListener::_enterBashCommand, &BaseListener::_exitBashCommand } },
		{ AST::NodeType::BashCommandSequence, { &BaseListener::_enterBashCommandSequence, &BaseListener::_exitBashCommandSequence } },
		{ AST::NodeType::BashForStatement, { &BaseListener::_enterBashForStatement, &BaseListener::_exitBashForStatement } },
		{ AST::NodeType::BashFunction, { &BaseListener::_enterBashFunction, &BaseListener::_exitBashFunction } },
		{ AST::NodeType::BashIfCondition, { &BaseListener::_enterBashIfCondition, &BaseListener::_exitBashIfCondition } },
		{ AST::NodeType::BashIfElseBranch, { &BaseListener::_enterBashIfElseBranch, &BaseListener::_exitBashIfElseBranch } },
		{ AST::NodeType::BashIfRootBranch, { &BaseListener::_enterBashIfRootBranch, &BaseListener::_exitBashIfRootBranch } },
		{ AST::NodeType::BashIfStatement, { &BaseListener::_enterBashIfStatement, &BaseListener::_exitBashIfStatement } },
		{ AST::NodeType::BashInCondition, { &BaseListener::_enterBashInCondition, &BaseListener::_exitBashInCondition } },
		{ AST::NodeType::BashPipeline, { &BaseListener::_enterBashPipeline, &BaseListener::_exitBashPipeline } },
		{ AST::NodeType::BashRedirection, { &BaseListener::_enterBashRedirection, &BaseListener::_exitBashRedirection } },
		{ AST::NodeType::BashSelectStatement, { &BaseListener::_enterBashSelectStatement, &BaseListener::_exitBashSelectStatement } },
		{ AST::NodeType::BashUntilStatement, { &BaseListener::_enterBashUntilStatement, &BaseListener::_exitBashUntilStatement } },
		{ AST::NodeType::BashVariable, { &BaseListener::_enterBashVariable, &BaseListener::_exitBashVariable } },
		{ AST::NodeType::BashWhileOrUntilCondition, { &BaseListener::_enterBashWhileOrUntilCondition, &BaseListener::_exitBashWhileOrUntilCondition } },
		{ AST::NodeType::BashWhileStatement, { &BaseListener::_enterBashWhileStatement, &BaseListener::_exitBashWhileStatement } },
		{ AST::NodeType::Block, { &BaseListener::_enterBlock, &BaseListener::_exitBlock } },
		{ AST::NodeType::ClassDefinition, { &BaseListener::_enterClassDefinition, &BaseListener::_exitClassDefinition } },
		{ AST::NodeType::ConstructorDefinition, { &BaseListener::_enterConstructorDefinition, &BaseListener::_exitConstructorDefinition } },
		{ AST::NodeType::DatamemberDeclaration, { &BaseListener::_enterDatamemberDeclaration, &BaseListener::_exitDatamemberDeclaration } },
		{ AST::NodeType::DeleteStatement, { &BaseListener::_enterDeleteStatement, &BaseListener::_exitDeleteStatement } },
		{ AST::NodeType::DestructorDefinition, { &BaseListener::_enterDestructorDefinition, &BaseListener::_exitDestructorDefinition } },
		{ AST::NodeType::DoublequotedString, { &BaseListener::_enterDoublequotedString, &BaseListener::_exitDoublequotedString } },
		{ AST::NodeType::DynamicCast, { &BaseListener::_enterDynamicCast, &BaseListener::_exitDynamicCast } },
		{ AST::NodeType::DynamicCastTarget, { &BaseListener::_enterDynamicCastTarget, &BaseListener::_exitDynamicCastTarget } },
		{ AST::NodeType::HeredocBody, { &BaseListener::_enterHeredocBody, &BaseListener::_exitHeredocBody } },
		{ AST::NodeType::HereString, { &BaseListener::_enterHereString, &BaseListener::_exitHereString } },
		{ AST::NodeType::IncludeStatement, { &BaseListener::_enterIncludeStatement, &BaseListener::_exitIncludeStatement } },
		{ AST::NodeType::MethodDefinition, { &BaseListener::_enterMethodDefinition, &BaseListener::_exitMethodDefinition } },
		{ AST::NodeType::NewStatement, { &BaseListener::_enterNewStatement, &BaseListener::_exitNewStatement } },
		{ AST::NodeType::ObjectAssignment, { &BaseListener::_enterObjectAssignment, &BaseListener::_exitObjectAssignment } },
		{ AST::NodeType::ObjectInstantiation, { &BaseListener::_enterObjectInstantiation, &BaseListener::_exitObjectInstantiation } },
		{ AST::NodeType::ObjectReference, { &BaseListener::_enterObjectReference, &BaseListener::_exitObjectReference } },
		{ AST::NodeType::ParameterExpansion, { &BaseListener::_enterParameterExpansion, &BaseListener::_exitParameterExpansion } },
		{ AST::NodeType::PointerDeclaration, { &BaseListener::_enterPointerDeclaration, &BaseListener::_exitPointerDeclaration } },
		{ AST::NodeType::PrimitiveAssignment, { &BaseListener::_enterPrimitiveAssignment, &BaseListener::_exitPrimitiveAssignment } },
		{ AST::NodeType::ProcessSubstitution, { &BaseListener::_enterProcessSubstitution, &BaseListener::_exitProcessSubstitution } },
		{ AST::NodeType::Program, { &BaseListener::_enterProgram, &BaseListener::_exitProgram } },
		{ AST::NodeType::RawSubshell, { &BaseListener::_enterRawSubshell, &BaseListener::_exitRawSubshell } },
		{ AST::NodeType::RawText, { &BaseListener::_enterRawText, &BaseListener::_exitRawText } },
		{ AST::NodeType::Rvalue, { &BaseListener::_enterRvalue, &BaseListener::_exitRvalue } },
		{ AST::NodeType::SubshellSubstitution, { &BaseListener::_enterSubshellSubstitution, &BaseListener::_exitSubshellSubstitution } },
		{ AST::NodeType::Supershell, { &BaseListener::_enterSupershell, &BaseListener::_exitSupershell } },
		{ AST::NodeType::TypeofExpression, { &BaseListener::_enterTypeofExpression, &BaseListener::_exitTypeofExpression } },
		{ AST::NodeType::ValueAssignment, { &BaseListener::_enterValueAssignment, &BaseListener::_exitValueAssignment } }
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
