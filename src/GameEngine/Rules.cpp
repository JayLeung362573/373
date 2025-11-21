#include "Rules.h"

VisitResult ast::Constant::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Variable::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Attribute::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Comparison::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::LogicalOperation::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::UnaryOperation::accept(ASTVisitor &visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Assignment::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Extend::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Reverse::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Shuffle::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Discard::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Sort::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::Match::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::InputText::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::InputChoice::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::InputRange::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

VisitResult ast::InputVote::accept(ast::ASTVisitor& visitor)
{
    return visitor.visit(*this);
};

std::unique_ptr<ast::Variable>
ast::makeVariable(Name name) {
    return std::make_unique<ast::Variable>(std::move(name));
}

std::unique_ptr<ast::Attribute>
ast::makeAttribute(std::unique_ptr<ast::Expression> expr, String attr)
{
    return std::make_unique<ast::Attribute>(std::move(expr), std::move(attr));
}

std::unique_ptr<ast::Constant>
ast::makeConstant(Value value) {
    return std::make_unique<ast::Constant>(value);
}

std::unique_ptr<ast::Comparison>
ast::makeComparison(std::unique_ptr<ast::Expression> left,
                    std::unique_ptr<ast::Expression> right,
                    ast::Comparison::Kind kind)
{
    return std::make_unique<ast::Comparison>(
        std::move(left), std::move(right), kind
    );
}

std::unique_ptr<ast::LogicalOperation>
ast::makeLogicalOperation(std::unique_ptr<ast::Expression> left,
                          std::unique_ptr<ast::Expression> right,
                          ast::LogicalOperation::Kind kind)
{
    return std::make_unique<ast::LogicalOperation>(
        std::move(left), std::move(right), kind
    );
}

std::unique_ptr<ast::UnaryOperation>
ast::makeUnaryOperation(std::unique_ptr<ast::Expression> target,
                        ast::UnaryOperation::Kind kind)
{
    return std::make_unique<ast::UnaryOperation>(std::move(target), kind);
}

std::unique_ptr<ast::Assignment>
ast::makeAssignment(std::unique_ptr<ast::Expression> targetExpr,
               std::unique_ptr<ast::Expression> valueToAssign)
{
    return std::make_unique<ast::Assignment>(
        std::move(targetExpr), std::move(valueToAssign)
    );
}

std::unique_ptr<ast::Extend>
ast::makeExtend(std::unique_ptr<ast::Expression> target,
                std::unique_ptr<ast::Expression> value)
{
    return std::make_unique<ast::Extend>(std::move(target), std::move(value));
}

std::unique_ptr<ast::Reverse>
ast::makeReverse(std::unique_ptr<ast::Expression> target)
{
    return std::make_unique<ast::Reverse>(std::move(target));
}

std::unique_ptr<ast::Shuffle>
ast::makeShuffle(std::unique_ptr<ast::Expression> target)
{
    return std::make_unique<ast::Shuffle>(std::move(target));
}

std::unique_ptr<ast::Discard>
ast::makeDiscard(std::unique_ptr<ast::Expression> target,
                 std::unique_ptr<ast::Expression> amount)
{
    return std::make_unique<ast::Discard>(std::move(target), std::move(amount));
}

std::unique_ptr<ast::Sort>
ast::makeSort(std::unique_ptr<ast::Expression> target, std::optional<String> key)
{
    return std::make_unique<ast::Sort>(std::move(target), key);
}

std::unique_ptr<ast::Match>
ast::makeMatch(std::unique_ptr<ast::Expression> target,
               std::vector<ast::Match::Candidate> candidates)
{
    return std::make_unique<ast::Match>(std::move(target), std::move(candidates));
}

std::unique_ptr<ast::InputText>
ast::makeInputText(std::unique_ptr<ast::Variable> playerVar,
                   std::unique_ptr<ast::Expression> targetExpr,
                   String prompt)
{
    return std::make_unique<ast::InputText>(
        std::move(playerVar), std::move(targetExpr), std::move(prompt)
    );
}

std::unique_ptr<ast::InputChoice>
ast::makeInputChoice(std::unique_ptr<ast::Variable> playerVar,
                     std::unique_ptr<ast::Expression> targetExpr,
                     String prompt,
                     std::unique_ptr<ast::Expression> choices)
{
    return std::make_unique<ast::InputChoice>(
        std::move(playerVar), std::move(targetExpr), std::move(prompt), std::move(choices)
    );
}

std::unique_ptr<ast::InputRange>
ast::makeInputRange(std::unique_ptr<ast::Variable> playerVar,
                    std::unique_ptr<ast::Expression> targetExpr,
                    String prompt,
                    std::unique_ptr<ast::Expression> minValue,
                    std::unique_ptr<ast::Expression> maxValue)
{
    return std::make_unique<ast::InputRange>(
        std::move(playerVar), std::move(targetExpr), std::move(prompt), std::move(minValue), std::move(maxValue)
    );
}

std::unique_ptr<ast::InputVote>
ast::makeInputVote(std::unique_ptr<ast::Variable> playerVar,
                   std::unique_ptr<ast::Expression> targetExpr,
                   String prompt,
                   std::unique_ptr<ast::Expression> choices)
{
    return std::make_unique<ast::InputVote>(
        std::move(playerVar), std::move(targetExpr), std::move(prompt), std::move(choices)
    );
}

std::unique_ptr<ast::Constant>
ast::cloneConstant(ast::Constant* constant)
{
    return std::make_unique<ast::Constant>(constant->getValue());
}

std::unique_ptr<ast::Variable>
ast::cloneVariable(ast::Variable* variable)
{
    return std::make_unique<ast::Variable>(variable->getName());
}

std::unique_ptr<ast::Attribute>
ast::cloneAttribute(ast::Attribute* attribute)
{
    return std::make_unique<ast::Attribute>(
        ast::cloneExpression(attribute->getBase()),
        attribute->getAttr()
    );
}

std::unique_ptr<ast::Expression>
ast::cloneExpression(ast::Expression* expr)
{
    if (auto variable = ast::castExpressionToVariable(expr))
    {
        return ast::cloneVariable(variable);
    }
    else if (auto constant = ast::castExpressionToConstant(expr))
    {
        return ast::cloneConstant(constant);
    }
    else if (auto attr = ast::castExpressionToAttribute(expr))
    {
        return ast::cloneAttribute(attr);
    }
    throw std::runtime_error("Unknown expression type in cloneExpression");
}

ast::Constant*
ast::castExpressionToConstant(ast::Expression* expr)
{
    return dynamic_cast<ast::Constant*>(expr);
}

ast::Variable*
ast::castExpressionToVariable(ast::Expression* expr)
{
    return dynamic_cast<ast::Variable*>(expr);
}

ast::Attribute*
ast::castExpressionToAttribute(ast::Expression* expr)
{
    return dynamic_cast<ast::Attribute*>(expr);
}
