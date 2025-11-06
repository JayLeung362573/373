#include <cassert>
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

    VisitResult baseResult = evaluateExpression(*baseExpr);
    if (!baseResult.isReference())
    {
        throw std::runtime_error("Expected expression to evaluate to a reference");
    }

    Value& baseValue = baseResult.getValue();
    Value& attrValue = baseValue.getAttribute(attribute.getAttr());
    return VisitResult{VisitResult::Status::Done, &attrValue};
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

    VisitResult baseResult = evaluateExpression(*baseExpr);
    if (!baseResult.isReference())
    {
        throw std::runtime_error("Expected expression to evaluate to a reference");
    }

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

Value
GameInterpreter::getPlayerAttribute(const ast::Variable& playerVar, String attr)
{
    auto playerAttr = ast::makeAttribute(ast::makeVariable(playerVar.getName()), attr);
    VisitResult result = playerAttr->accept(*this);

    if (!result.hasValue())
    {
        throw std::runtime_error(
              "Failed to get player attribute: " + attr.value
//            std::format("Failed to get player attribute: {}", attr.value)
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
