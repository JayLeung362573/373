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
    return VisitResult{value};
}

VisitResult
GameInterpreter::visit(const ast::Variable& variable)
{
    Value* value = m_variableMap.load(variable.getName());
    return VisitResult{value};
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

    return VisitResult{&attrValue};
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

    return VisitResult{Value{boolResult}};
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

    return VisitResult{Value{boolResult}};
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

    return VisitResult{Value{boolResult}};
}

VisitResult
GameInterpreter::visit(const ast::ArithmeticOperation& arithmeticOp)
{
    Value left = evaluateExpression(*arithmeticOp.getLeft()).getValue();
    Value right = evaluateExpression(*arithmeticOp.getRight()).getValue();

    Value result;

    switch (arithmeticOp.getKind())
    {
        case ast::ArithmeticOperation::Kind::ADD: result = doArithmeticAdd(left, right); break;
    }

    return VisitResult{result};
}

VisitResult
GameInterpreter::visit(const ast::Callable& callable)
{
    Value result;

    switch (callable.getKind())
    {
        case ast::Callable::Kind::SIZE: result = callSizeBuiltin(callable); break;
        case ast::Callable::Kind::UP_FROM: result = callUpFromBuiltin(callable); break;
        default: throw std::runtime_error("Unknown callable kind");
    }

    return VisitResult{result};
}

Value
GameInterpreter::callSizeBuiltin(const ast::Callable& callable)
{
    auto args = callable.getArgs();

    if (args.size() != 0)
    {
        throw std::runtime_error(
            std::format("size() expects 0 args, got {}", args.size())
        );
    }

    List<Value> list = evaluateExpression(*callable.getLeft()).getValue().asList();

    return Value{Integer{static_cast<int>(list.size())}};
}

Value
GameInterpreter::callUpFromBuiltin(const ast::Callable& callable)
{
    auto args = callable.getArgs();

    if (callable.getArgs().size() != 1)
    {
        throw std::runtime_error(
            std::format("upfrom() expects 1 arg, got {}", args.size())
        );
    }
    Integer fromParam = evaluateExpression(*args[0]).getValue().asInteger();
    Integer toParam = evaluateExpression(*callable.getLeft()).getValue().asInteger();

    return Value{upFrom(fromParam.value, toParam.value)};
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
    return {};
}

VisitResult
GameInterpreter::visit(const ast::Extend& extend)
{
    VisitResult targetResult = resolveExpression(*extend.getTarget());
    Value& target = targetResult.getValue();

    VisitResult valueResult = evaluateExpression(*extend.getValue());
    Value value = valueResult.getValue();

    target.asList().extend(value.asList());

    return {};
}

VisitResult
GameInterpreter::visit(const ast::Reverse& reverse)
{
    VisitResult targetResult = resolveExpression(*reverse.getTarget());
    Value& target = targetResult.getValue();

    target.asList().reverse();

    return {};
}

VisitResult
GameInterpreter::visit(const ast::Shuffle& shuffle)
{
    VisitResult targetResult = resolveExpression(*shuffle.getTarget());
    Value& target = targetResult.getValue();

    target.asList().shuffle();

    return {};
}

VisitResult
GameInterpreter::visit(const ast::Discard& discard)
{
    VisitResult targetResult = resolveExpression(*discard.getTarget());
    Value& target = targetResult.getValue();

    VisitResult amountResult = evaluateExpression(*discard.getAmount());
    Value amount = amountResult.getValue();

    target.asList().discard(amount.asInteger());

    return {};
}

VisitResult
GameInterpreter::visit(const ast::Sort& sort)
{
    VisitResult targetResult = resolveExpression(*sort.getTarget());
    Value& target = targetResult.getValue();

    List<Value> sortedTarget = sortList(target.asList(), sort.getKey());

    target = Value{sortedTarget};

    return {};
}

VisitResult
GameInterpreter::visit(const ast::Match& match)
{
    auto ctx = getCurrentMatchExecutionContext();
    bool isFirstVisit = !ctx.has_value();

    if (isFirstVisit)
    {
        auto maybeCandidate = findMatch(match);
        if (!maybeCandidate.has_value())
        {
            // No match, we're done
            return {};
        }

        auto iterator = std::make_unique<ProgramIterator>(
            ProgramRaw{{maybeCandidate.value().statements}}
        );
        setCurrentStatementContext(
            ProgramIterator::MatchExecutionContext{std::move(iterator)}
        );
    }

    ctx = getCurrentMatchExecutionContext();
    if (!ctx.has_value())
    {
        throw std::runtime_error("Match execution context not found");
    }

    executeProgram(*(ctx.value()->iterator));

    return {};
}

std::optional<ast::Match::CandidateRaw>
GameInterpreter::findMatch(const ast::Match& match)
{
    Value targetValue = evaluateExpression(*match.getTarget()).getValue();

    for (auto& candidate : match.getCandidates())
    {
        Value candidateValue = evaluateExpression(*(candidate.expressionCandidate)).getValue();
        if (isEqual(targetValue, candidateValue).value)
        {
            return candidate;
        }
    }

    return std::nullopt;
}

VisitResult
GameInterpreter::visit(const ast::ForLoop& forLoop)
{
    auto ctx = getCurrentForLoopExecutionContext();
    bool isFirstVisit = !ctx.has_value();

    List<Value> target = evaluateExpression(*forLoop.getTarget())
                        .getValue()
                        .asList();

    if (isFirstVisit)
    {
        auto iterator = std::make_unique<ProgramIterator>(
            ProgramRaw{{forLoop.getStatements()}}
        );
        setCurrentStatementContext(
            ProgramIterator::ForLoopExecutionContext{std::move(iterator)}
        );
    }

    ctx = getCurrentForLoopExecutionContext();
    if (!ctx.has_value())
    {
        throw std::runtime_error("ForLoop execution context not found");
    }

    while (ctx.value()->listIndex < target.size() && !needsIO())
    {
        doVariableAssignment(*forLoop.getElement(), target.atIndex(ctx.value()->listIndex));

        executeProgram(*(ctx.value()->iterator));

        if (ctx.value()->iterator->isDone())
        {
            ctx.value()->iterator->reset();
            ctx.value()->listIndex++;
        }
    }

    if (ctx.value()->listIndex == target.size())
    {
        // done, clean up
        deleteVariable(*forLoop.getElement());
    }

    return {};
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

void
GameInterpreter::storeVariable(const Name& name, Value value) {
    m_variableMap.store(name, value);
}

void
GameInterpreter::deleteVariable(ast::Variable& variable)
{
    m_variableMap.del(variable.getName());
}

void
GameInterpreter::execute()
{
    m_waitingForInput = false;
    if (!m_program.has_value())
    {
        throw std::runtime_error("No program to execute");
    }

    executeProgram(*m_iterator.get());
}

void
GameInterpreter::executeProgram(ProgramIterator& iterator)
{
    while (iterator.currentStatement() != nullptr && !needsIO())
    {
        m_currentIterator = &iterator;
        iterator.currentStatement()->accept(*this);

        if (!needsIO())
        {
            iterator.goNext();
        }
    }
}

void
GameInterpreter::assertCurrentIterator()
{
    if (m_currentIterator == nullptr)
    {
        throw std::runtime_error(
            "Expected m_currentIterator to be set. Is there a program to execute?"
        );
    }
}

std::optional<ProgramIterator::ForLoopExecutionContext*>
GameInterpreter::getCurrentForLoopExecutionContext()
{
    assertCurrentIterator();
    return m_currentIterator->currentContext<ProgramIterator::ForLoopExecutionContext>();
}

std::optional<ProgramIterator::MatchExecutionContext*>
GameInterpreter::getCurrentMatchExecutionContext()
{
    assertCurrentIterator();
    return m_currentIterator->currentContext<ProgramIterator::MatchExecutionContext>();
}

void
GameInterpreter::setCurrentStatementContext(ProgramIterator::StatementContext ctx)
{
    m_currentIterator->setCurrentContext(std::move(ctx));
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

bool
GameInterpreter::needsIO() const
{
    return m_inputManager.getPendingRequests().size() > 0 || m_waitingForInput;
}

bool
GameInterpreter::isDone() const {
    if(!m_program.has_value()) return true;

    if(!m_iterator) return true;

    return m_iterator->currentStatement() == nullptr;
}

VisitResult
GameInterpreter::visit(const ast::InputText& inputText)
{
    auto playerVar = inputText.getPlayer();
    auto targetExpr = inputText.getTarget();
    String prompt = inputText.getPrompt();
    String playerID = getPlayerAttribute(*playerVar, String{"id"}).asString();

    auto maybeText = m_inputManager.getTextInput(playerID, prompt);
    if (!maybeText)
    {
        m_waitingForInput = true;
        return {};
    }

    m_waitingForInput = false;

    Value input{*maybeText};
    auto assignment = ast::makeAssignment(
        ast::cloneExpression(targetExpr),
        ast::makeConstant(input)
    );
    assignment->accept(*this);

    return {};
}

VisitResult
GameInterpreter::visit(const ast::InputChoice& inputChoice)
{
    auto playerVar = inputChoice.getPlayer();
    auto targetExpr = inputChoice.getTarget();
    String prompt = inputChoice.getPrompt();
    auto choicesExpr = inputChoice.getChoices();
    String playerID = getPlayerAttribute(*playerVar, String{"id"}).asString();

    VisitResult choicesResult = evaluateExpression(*choicesExpr);
    Value& choicesValue = choicesResult.getValue();

    if (!choicesValue.isList())
    {
        throw std::runtime_error("Choices must evaluate to a list");
    }

    List<Value> choices = choicesValue.asList();

    auto maybeChoice = m_inputManager.getChoiceInput(playerID, prompt, choices);
    if (!maybeChoice)
    {
        m_waitingForInput = true;
        return {};
    }
    m_waitingForInput = false;

    Value choiceValue{*maybeChoice};
    auto assignment = ast::makeAssignment(
        ast::cloneExpression(targetExpr),
        ast::makeConstant(choiceValue)
    );
    assignment->accept(*this);

    return {};
}

VisitResult
GameInterpreter::visit(const ast::InputRange& inputRange)
{
    auto playerVar = inputRange.getPlayer();
    auto targetExpr = inputRange.getTarget();
    String prompt = inputRange.getPrompt();
    auto minExpr = inputRange.getMinValue();
    auto maxExpr = inputRange.getMaxValue();

    String playerID = getPlayerAttribute(*playerVar, String{"id"}).asString();

    Value minValue = evaluateExpression(*minExpr).getValue();
    Value maxValue = evaluateExpression(*maxExpr).getValue();

    auto maybeRange = m_inputManager.getRangeInput(
        playerID, prompt, minValue.asInteger(), maxValue.asInteger()
    );

    if (!maybeRange)
    {
        m_waitingForInput = true;
        return {};
    }
    m_waitingForInput = false;

    auto assignment = ast::makeAssignment(
        ast::cloneExpression(targetExpr),
        ast::makeConstant(Value{*maybeRange})
    );
    assignment->accept(*this);

    return {};
}

VisitResult
GameInterpreter::visit(const ast::InputVote& inputVote)
{
    auto playerVar = inputVote.getPlayer();
    auto targetExpr = inputVote.getTarget();
    String prompt = inputVote.getPrompt();
    auto choicesExpr = inputVote.getChoices();

    String playerID = getPlayerAttribute(*playerVar, String{"id"}).asString();

    VisitResult choicesResult = evaluateExpression(*choicesExpr);
    Value& choicesValue = choicesResult.getValue();

    if (!choicesValue.isList())
    {
        throw std::runtime_error("Vote choices must evaluate to a list");
    }

    List<Value> choices = choicesValue.asList();

    auto maybeVote = m_inputManager.getVoteInput(playerID, prompt, choices);

    if (!maybeVote)
    {
        m_waitingForInput = true;
        return {};
    }
    m_waitingForInput = false;

    Value voteValue{*maybeVote};
    auto assignment = ast::makeAssignment(
        ast::cloneExpression(targetExpr),
        ast::makeConstant(voteValue)
    );
    assignment->accept(*this);

    return {};
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
        );
    }
    return result.getValue();
}
