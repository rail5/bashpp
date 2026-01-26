/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/Nodes/ArrayAssignment.h>
#include <AST/Nodes/ArrayIndex.h>
#include <AST/Nodes/Bash53NativeSupershell.h>
#include <AST/Nodes/BashArithmeticForCondition.h>
#include <AST/Nodes/BashArithmeticForStatement.h>
#include <AST/Nodes/BashArithmeticStatement.h>
#include <AST/Nodes/BashArithmeticSubstitution.h>
#include <AST/Nodes/BashCaseInput.h>
#include <AST/Nodes/BashCasePattern.h>
#include <AST/Nodes/BashCasePatternHeader.h>
#include <AST/Nodes/BashCaseStatement.h>
#include <AST/Nodes/BashCommand.h>
#include <AST/Nodes/BashCommandSequence.h>
#include <AST/Nodes/BashForStatement.h>
#include <AST/Nodes/BashIfCondition.h>
#include <AST/Nodes/BashIfElseBranch.h>
#include <AST/Nodes/BashIfRootBranch.h>
#include <AST/Nodes/BashIfStatement.h>
#include <AST/Nodes/BashInCondition.h>
#include <AST/Nodes/BashPipeline.h>
#include <AST/Nodes/BashRedirection.h>
#include <AST/Nodes/BashSelectStatement.h>
#include <AST/Nodes/BashTestConditionCommand.h>
#include <AST/Nodes/BashUntilStatement.h>
#include <AST/Nodes/BashVariable.h>
#include <AST/Nodes/BashWhileOrUntilCondition.h>
#include <AST/Nodes/BashWhileStatement.h>
#include <AST/Nodes/BashFunction.h>
#include <AST/Nodes/Block.h>
#include <AST/Nodes/ClassDefinition.h>
#include <AST/Nodes/Connective.h>
#include <AST/Nodes/ConstructorDefinition.h>
#include <AST/Nodes/DatamemberDeclaration.h>
#include <AST/Nodes/DeleteStatement.h>
#include <AST/Nodes/DestructorDefinition.h>
#include <AST/Nodes/DoublequotedString.h>
#include <AST/Nodes/DynamicCast.h>
#include <AST/Nodes/DynamicCastTarget.h>
#include <AST/Nodes/HeredocBody.h>
#include <AST/Nodes/HereString.h>
#include <AST/Nodes/IncludeStatement.h>
#include <AST/Nodes/MethodDefinition.h>
#include <AST/Nodes/NewStatement.h>
#include <AST/Nodes/ObjectAssignment.h>
#include <AST/Nodes/ObjectInstantiation.h>
#include <AST/Nodes/ObjectReference.h>
#include <AST/Nodes/ParameterExpansion.h>
#include <AST/Nodes/PointerDeclaration.h>
#include <AST/Nodes/PrimitiveAssignment.h>
#include <AST/Nodes/ProcessSubstitution.h>
#include <AST/Nodes/Program.h>
#include <AST/Nodes/RawSubshell.h>
#include <AST/Nodes/RawText.h>
#include <AST/Nodes/Rvalue.h>
#include <AST/Nodes/StringType.h>
#include <AST/Nodes/SubshellSubstitution.h>
#include <AST/Nodes/Supershell.h>
#include <AST/Nodes/TypeofExpression.h>
#include <AST/Nodes/ValueAssignment.h>
