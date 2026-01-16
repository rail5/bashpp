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
 * 
 */
class BaseListener {
	private:
		frozen::unordered_map<
			AST::NodeType,
			std::pair<
				std::function<void(std::shared_ptr<AST::ASTNode>)>,
				std::function<void(std::shared_ptr<AST::ASTNode>)>
			>,
			53
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

		virtual void enterProgram(std::shared_ptr<AST::Program> node) {}
		virtual void exitProgram(std::shared_ptr<AST::Program> node) {}
		virtual void enterArrayIndex(std::shared_ptr<AST::ArrayIndex> node) {}
		virtual void exitArrayIndex(std::shared_ptr<AST::ArrayIndex> node) {}
		virtual void enterBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node) {}
		virtual void exitBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node) {}
		virtual void enterBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node) {}
		virtual void exitBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node) {}
		virtual void enterBashArithmeticStatement(std::shared_ptr<AST::BashArithmeticStatement> node) {}
		virtual void exitBashArithmeticStatement(std::shared_ptr<AST::BashArithmeticStatement> node) {}
		virtual void enterBashCaseInput(std::shared_ptr<AST::BashCaseInput> node) {}
		virtual void exitBashCaseInput(std::shared_ptr<AST::BashCaseInput> node) {}
		virtual void enterBashCasePatternAction(std::shared_ptr<AST::BashCasePatternAction> node) {}
		virtual void exitBashCasePatternAction(std::shared_ptr<AST::BashCasePatternAction> node) {}
		virtual void enterBashCasePattern(std::shared_ptr<AST::BashCasePattern> node) {}
		virtual void exitBashCasePattern(std::shared_ptr<AST::BashCasePattern> node) {}
		virtual void enterBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node) {}
		virtual void exitBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node) {}
		virtual void enterBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node) {}
		virtual void exitBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node) {}
		virtual void enterBashCommand(std::shared_ptr<AST::BashCommand> node) {}
		virtual void exitBashCommand(std::shared_ptr<AST::BashCommand> node) {}
		virtual void enterBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) {}
		virtual void exitBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) {}
		virtual void enterBashForStatement(std::shared_ptr<AST::BashForStatement> node) {}
		virtual void exitBashForStatement(std::shared_ptr<AST::BashForStatement> node) {}
		virtual void enterBashIfCondition(std::shared_ptr<AST::BashIfCondition> node) {}
		virtual void exitBashIfCondition(std::shared_ptr<AST::BashIfCondition> node) {}
		virtual void enterBashIfElseBranch(std::shared_ptr<AST::BashIfElseBranch> node) {}
		virtual void exitBashIfElseBranch(std::shared_ptr<AST::BashIfElseBranch> node) {}
		virtual void enterBashIfRootBranch(std::shared_ptr<AST::BashIfRootBranch> node) {}
		virtual void exitBashIfRootBranch(std::shared_ptr<AST::BashIfRootBranch> node) {}
		virtual void enterBashIfStatement(std::shared_ptr<AST::BashIfStatement> node) {}
		virtual void exitBashIfStatement(std::shared_ptr<AST::BashIfStatement> node) {}
		virtual void enterBashInCondition(std::shared_ptr<AST::BashInCondition> node) {}
		virtual void exitBashInCondition(std::shared_ptr<AST::BashInCondition> node) {}
		virtual void enterBashPipeline(std::shared_ptr<AST::BashPipeline> node) {}
		virtual void exitBashPipeline(std::shared_ptr<AST::BashPipeline> node) {}
		virtual void enterBashRedirection(std::shared_ptr<AST::BashRedirection> node) {}
		virtual void exitBashRedirection(std::shared_ptr<AST::BashRedirection> node) {}
		virtual void enterBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node) {}
		virtual void exitBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node) {}
		virtual void enterBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node) {}
		virtual void exitBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node) {}
		virtual void enterBashVariable(std::shared_ptr<AST::BashVariable> node) {}
		virtual void exitBashVariable(std::shared_ptr<AST::BashVariable> node) {}
		virtual void enterBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node) {}
		virtual void exitBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node) {}
		virtual void enterBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node) {}
		virtual void exitBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node) {}
		virtual void enterBlock(std::shared_ptr<AST::Block> node) {}
		virtual void exitBlock(std::shared_ptr<AST::Block> node) {}
		virtual void enterClassDefinition(std::shared_ptr<AST::ClassDefinition> node) {}
		virtual void exitClassDefinition(std::shared_ptr<AST::ClassDefinition> node) {}
		virtual void enterConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node) {}
		virtual void exitConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node) {}
		virtual void enterDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) {}
		virtual void exitDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) {}
		virtual void enterDeleteStatement(std::shared_ptr<AST::DeleteStatement> node) {}
		virtual void exitDeleteStatement(std::shared_ptr<AST::DeleteStatement> node) {}
		virtual void enterDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node) {}
		virtual void exitDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node) {}
		virtual void enterDoublequotedString(std::shared_ptr<AST::DoublequotedString> node) {}
		virtual void exitDoublequotedString(std::shared_ptr<AST::DoublequotedString> node) {}
		virtual void enterDynamicCast(std::shared_ptr<AST::DynamicCast> node) {}
		virtual void exitDynamicCast(std::shared_ptr<AST::DynamicCast> node) {}
		virtual void enterDynamicCastTarget(std::shared_ptr<AST::DynamicCastTarget> node) {}
		virtual void exitDynamicCastTarget(std::shared_ptr<AST::DynamicCastTarget> node) {}
		virtual void enterHeredocBody(std::shared_ptr<AST::HeredocBody> node) {}
		virtual void exitHeredocBody(std::shared_ptr<AST::HeredocBody> node) {}
		virtual void enterHereString(std::shared_ptr<AST::HereString> node) {}
		virtual void exitHereString(std::shared_ptr<AST::HereString> node) {}
		virtual void enterIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) {}
		virtual void exitIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) {}
		virtual void enterMethodDefinition(std::shared_ptr<AST::MethodDefinition> node) {}
		virtual void exitMethodDefinition(std::shared_ptr<AST::MethodDefinition> node) {}
		virtual void enterNewStatement(std::shared_ptr<AST::NewStatement> node) {}
		virtual void exitNewStatement(std::shared_ptr<AST::NewStatement> node) {}
		virtual void enterObjectAssignment(std::shared_ptr<AST::ObjectAssignment> node) {}
		virtual void exitObjectAssignment(std::shared_ptr<AST::ObjectAssignment> node) {}
		virtual void enterObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node) {}
		virtual void exitObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node) {}
		virtual void enterObjectReference(std::shared_ptr<AST::ObjectReference> node) {}
		virtual void exitObjectReference(std::shared_ptr<AST::ObjectReference> node) {}
		virtual void enterParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node) {}
		virtual void exitParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node) {}
		virtual void enterPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node) {}
		virtual void exitPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node) {}
		virtual void enterPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) {}
		virtual void exitPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) {}
		virtual void enterProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) {}
		virtual void exitProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) {}
		virtual void enterRawSubshell(std::shared_ptr<AST::RawSubshell> node) {}
		virtual void exitRawSubshell(std::shared_ptr<AST::RawSubshell> node) {}
		virtual void enterRawText(std::shared_ptr<AST::RawText> node) {}
		virtual void exitRawText(std::shared_ptr<AST::RawText> node) {}
		virtual void enterRvalue(std::shared_ptr<AST::Rvalue> node) {}
		virtual void exitRvalue(std::shared_ptr<AST::Rvalue> node) {}
		virtual void enterSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) {}
		virtual void exitSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) {}
		virtual void enterSupershell(std::shared_ptr<AST::Supershell> node) {}
		virtual void exitSupershell(std::shared_ptr<AST::Supershell> node) {}
		virtual void enterTypeofExpression(std::shared_ptr<AST::TypeofExpression> node) {}
		virtual void exitTypeofExpression(std::shared_ptr<AST::TypeofExpression> node) {}
		virtual void enterValueAssignment(std::shared_ptr<AST::ValueAssignment> node) {}
		virtual void exitValueAssignment(std::shared_ptr<AST::ValueAssignment> node) {}
};

} // namespace AST
