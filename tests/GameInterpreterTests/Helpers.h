#include <gtest/gtest.h>
#include <optional>
#include <iostream>
#include "GameInterpreter.h"

inline void
doAssignment(GameInterpreter& interpreter, std::unique_ptr<ast::Assignment> assignment)
{
    VisitResult result = assignment->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

inline VisitResult
doComparison(GameInterpreter& interpreter, std::unique_ptr<ast::Comparison> comparison)
{
    VisitResult result = comparison->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_TRUE(result.hasValue());
    EXPECT_FALSE(result.isReference());

    return result;
}

inline VisitResult
doLogicalOperation(GameInterpreter& interpreter, std::unique_ptr<ast::LogicalOperation> logicalOp)
{
    VisitResult result = logicalOp->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_TRUE(result.hasValue());
    EXPECT_FALSE(result.isReference());

    return result;
}

inline VisitResult
doUnaryOperation(GameInterpreter& interpreter, std::unique_ptr<ast::UnaryOperation> unaryOp)
{
    VisitResult result = unaryOp->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_TRUE(result.hasValue());
    EXPECT_FALSE(result.isReference());

    return result;
}

inline void
doExtend(GameInterpreter& interpreter, std::unique_ptr<ast::Extend> extend)
{
    VisitResult result = extend->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

inline void
doReverse(GameInterpreter& interpreter, std::unique_ptr<ast::Reverse> reverse)
{
    VisitResult result = reverse->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

inline void
doShuffle(GameInterpreter& interpreter, std::unique_ptr<ast::Shuffle> shuffle)
{
    VisitResult result = shuffle->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

inline void
doDiscard(GameInterpreter& interpreter, std::unique_ptr<ast::Discard> discard)
{
    VisitResult result = discard->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

inline void
doSort(GameInterpreter& interpreter, std::unique_ptr<ast::Sort> sort)
{
    VisitResult result = sort->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

inline Value
loadVariable(GameInterpreter& interpreter, Name targetName)
{
    ast::Variable loadVariable(targetName);
    VisitResult result = loadVariable.accept(interpreter);

    EXPECT_TRUE(result.isDone());
    EXPECT_TRUE(result.hasValue());
    EXPECT_TRUE(result.isReference());

    return result.getValue();
}
