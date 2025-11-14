#include <memory>
#include <vector>
#include <variant>
#include <optional>

#include "GameInterpreter.h"

VisitResult
GameInterpreter::visit(const ast::ASTNode& node)
{
    throw std::runtime_error("Invalid node during evaluation");
}

VisitResult
GameInterpreter::visit(const ast::Constant& constant)
{
    Value value = constant.getValue();
    return VisitResult{VisitResult::Status::Done, value};
}

VisitResult
GameInterpreter::visit(const ast::Variable& variable)
{
    Value* value = m_variableMap.load(variable.getName());
    return VisitResult{VisitResult::Status::Done, value};
}

VisitResult
GameInterpreter::visit(const ast::Attribute& attribute)
{
    auto baseExpr = attribute.getBase();
    if (!baseExpr)
    {
        throw std::runtime_error("Attribute base cannot be null");
    }

    if(!castExpressionToVariable(baseExpr)
       && !castExpressionToAttribute(baseExpr))
    {
        throw std::runtime_error("Attribute base must be a Variable or Attribute");
    }

    VisitResult baseResult = resolveExpression(*baseExpr);
    Value& baseValue = baseResult.getValue();
    Value& attrValue = baseValue.getAttribute(attribute.getAttr());

    return VisitResult{VisitResult::Status::Done, &attrValue};
}

VisitResult
GameInterpreter::visit(const ast::Comparison& comparison)
{
    Value left = evaluateExpression(*comparison.getLeft()).getValue();
    Value right = evaluateExpression(*comparison.getRight()).getValue();

    Boolean boolResult;

    switch (comparison.getKind())
    {
        case ast::Comparison::Kind::EQ: boolResult = isEqual(left, right); break;
        case ast::Comparison::Kind::LT: boolResult = isLessThan(left, right); break;
    }

    return VisitResult{VisitResult::Status::Done, Value{boolResult}};
}

VisitResult
GameInterpreter::visit(const ast::LogicalOperation& logicalOp)
{
    Value left = evaluateExpression(*logicalOp.getLeft()).getValue();
    Value right = evaluateExpression(*logicalOp.getRight()).getValue();

    Boolean boolResult;

    switch (logicalOp.getKind())
    {
        case ast::LogicalOperation::Kind::OR: boolResult = Boolean{doLogicalOr(left, right)}; break;
    }

    return VisitResult{VisitResult::Status::Done, Value{boolResult}};
}

VisitResult
GameInterpreter::visit(const ast::UnaryOperation& unaryOp)
{
    Value target = evaluateExpression(*unaryOp.getTarget()).getValue();

    Boolean boolResult;

    switch (unaryOp.getKind())
    {
        case ast::UnaryOperation::Kind::NOT: boolResult = Boolean{doUnaryNot(target)}; break;
    }

    return VisitResult{VisitResult::Status::Done, Value{boolResult}};
}

VisitResult
GameInterpreter::visit(const ast::Assignment& assignment)
{
    Value valueToAssign = assignment.getValue()->accept(*this).getValue();
    auto targetExpr = assignment.getTarget();

    if (auto varTarget = castExpressionToVariable(targetExpr))
    {
        doVariableAssignment(*varTarget, valueToAssign);
    }
    else if (auto attrTarget = castExpressionToAttribute(targetExpr))
    {
        doAttributeAssignment(*attrTarget, valueToAssign);
    }
    else
    {
        throw std::runtime_error("Assignment target must be a Variable or an Attribute");
    }
    return VisitResult{VisitResult::Status::Done, {}};
}

VisitResult
GameInterpreter::visit(const ast::Extend& extend)
{
    VisitResult targetResult = resolveExpression(*extend.getTarget());
    Value& target = targetResult.getValue();

    VisitResult valueResult = evaluateExpression(*extend.getValue());
    Value value = valueResult.getValue();

    target.asList().extend(value.asList());

    return VisitResult{VisitResult::Status::Done, {}};
}

VisitResult
GameInterpreter::visit(const ast::Reverse& reverse)
{
    VisitResult targetResult = resolveExpression(*reverse.getTarget());
    Value& target = targetResult.getValue();

    target.asList().reverse();

    return VisitResult{VisitResult::Status::Done, {}};
}

VisitResult
GameInterpreter::visit(const ast::Shuffle& shuffle)
{
    VisitResult targetResult = resolveExpression(*shuffle.getTarget());
    Value& target = targetResult.getValue();

    target.asList().shuffle();

    return VisitResult{VisitResult::Status::Done, {}};
}

VisitResult
GameInterpreter::visit(const ast::Discard& discard)
{
    VisitResult targetResult = resolveExpression(*discard.getTarget());
    Value& target = targetResult.getValue();

    VisitResult amountResult = evaluateExpression(*discard.getAmount());
    Value amount = amountResult.getValue();

    target.asList().discard(amount.asInteger());

    return VisitResult{VisitResult::Status::Done, {}};
}

VisitResult
GameInterpreter::visit(const ast::Sort& sort)
{
    VisitResult targetResult = resolveExpression(*sort.getTarget());
    Value& target = targetResult.getValue();

    List<Value> sortedTarget = sortList(target.asList(), sort.getKey());

    target = Value{sortedTarget};

    return VisitResult{VisitResult::Status::Done, {}};
}

void
GameInterpreter::doVariableAssignment(ast::Variable& varTarget, Value valueToAssign)
{
    m_variableMap.store(varTarget.getName(), valueToAssign);
}

void
GameInterpreter::doAttributeAssignment(ast::Attribute& attrTarget, Value valueToAssign)
{
    auto baseExpr = attrTarget.getBase();
    if (!baseExpr)
    {
        throw std::runtime_error("Attribute base cannot be null");
    }

    VisitResult baseResult = resolveExpression(*baseExpr);
    Value& baseValue = baseResult.getValue();
    baseValue.setAttribute(attrTarget.getAttr(), valueToAssign);
}

VisitResult
GameInterpreter::evaluateExpression(ast::Expression& expr)
{
    VisitResult result = expr.accept(*this);
    if (!result.hasValue())
    {
        throw std::runtime_error("Expression did not evaluate to a value");
    }
    return result;
}

VisitResult
GameInterpreter::resolveExpression(ast::Expression& expr)
{
    VisitResult result = expr.accept(*this);
    if (!result.hasValue())
    {
        throw std::runtime_error("Expression did not resolve to a Value");
    }
    if (!result.isReference())
    {
        throw std::runtime_error("Expression did not resolve to a Value reference");
    }

    return result;
}

Boolean
GameInterpreter::isEqual(const Value& a, const Value& b)
{
    bool isEqual = (a == b);
    return Boolean{isEqual};
}

Boolean
GameInterpreter::isLessThan(const Value& left, const Value& right)
{
    auto maybeIsLessThan = maybeCompareValues(left, right);
    if (maybeIsLessThan.has_value())
    {
        return Boolean{*maybeIsLessThan};
    }
    throw std::runtime_error("Values are not less-than comparable");
}

VisitResult
GameInterpreter::visit(const ast::InputTextStatement& inputTextStatement)
{
    auto playerVar = inputTextStatement.getPlayer();
    auto targetExpr = inputTextStatement.getTarget();
    String prompt = inputTextStatement.getPrompt();
    String playerID = getPlayerAttribute(*playerVar, String{"id"}).asString();

    auto maybeInput = getTextInputMsg(playerID, prompt);
    if (!maybeInput)
    {
        m_outGameMessages.push_back(
            GameMessage{GetTextInputMessage{playerID, prompt}}
        );
        return VisitResult{VisitResult::Status::Pending, {}};
    }

    Value input{String{maybeInput->input}};
    auto assignment = ast::makeAssignment(
        ast::cloneExpression(targetExpr),
        ast::makeConstant(input)
    );
    assignment->accept(*this);

    return VisitResult{VisitResult::Status::Done, {}};
}

VisitResult
GameInterpreter::visit(const ast::Match& match)
{
    // Evaluate the target expression
    auto* targetExpr = match.getTarget();
    if (!targetExpr)
    {
        throw std::runtime_error("Match target cannot be null");
    }

    VisitResult targetResult = evaluateExpression(*targetExpr);
    if (!targetResult.hasValue())
    {
        throw std::runtime_error("Match target must evaluate to a value");
    }

    Value& targetValue = targetResult.getValue();

    // Get all candidate pairs
    auto pairs = match.getExpressionCandidateAndStatementsPairs();

    // Find the first matching case
    for (const auto& pair : pairs)
    {
        if (!pair.expressionCandidate)
        {
            // Default case (no expression to match) - always matches
            for (auto* statement : pair.statements)
            {
                if (statement) statement->accept(*this);
            }
            return VisitResult{VisitResult::Status::Done, {}};
        }

        // Evaluate the candidate expression
        VisitResult candidateResult = evaluateExpression(*pair.expressionCandidate);
        if (candidateResult.hasValue())
        {
            Value& candidateValue = candidateResult.getValue();

            // Check if they match
            if (isEqual(targetValue, candidateValue).value)
            {
                // Execute statements for this case
                for (auto* statement : pair.statements)
                {
                    if (statement) statement->accept(*this);
                }
                return VisitResult{VisitResult::Status::Done, {}};
            }
        }
    }

    // No case matched - do nothing
    return VisitResult{VisitResult::Status::Done, {}};
}

Value
GameInterpreter::getPlayerAttribute(const ast::Variable& playerVar, String attr)
{
    auto playerAttr = ast::makeAttribute(ast::makeVariable(playerVar.getName()), attr);
    VisitResult result = playerAttr->accept(*this);

    if (!result.hasValue())
    {
        throw std::runtime_error(
            std::format("Failed to get player attribute: {}", attr.value)
        );
    }
    return result.getValue();
}

std::optional<TextInputMessage>
GameInterpreter::getTextInputMsg(String playerID, String prompt) const
{
    for (const auto& msg : m_inGameMessages)
    {
        if (const auto* inputMsg = std::get_if<TextInputMessage>(&msg.inner))
        {
            if (inputMsg->playerID == playerID && inputMsg->prompt == prompt)
            {
                return *inputMsg;
            }
        }
    }
    return std::nullopt;
}

void
GameInterpreter::setInGameMessages(const std::vector<GameMessage>& inGameMessages)
{
    m_inGameMessages = inGameMessages;
}

std::vector<GameMessage>
GameInterpreter::consumeOutGameMessages()
{
    auto out = std::move(m_outGameMessages);
    m_outGameMessages.clear();
    return out;
}
