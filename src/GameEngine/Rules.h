#pragma once

#include <cassert>
#include <memory>
#include <optional>

#include "Types.h"


struct VisitResult
{
    enum class Status
    {
        Done,
        Pending
    };

    Status status;
    std::optional<std::variant<Value, Value*>> value;

    bool hasValue() const { return value.has_value(); }

    Value& getValue()
    {
        assert(hasValue());
        if (std::holds_alternative<Value*>(*value)) { return *std::get<Value*>(*value); }
        else { return std::get<Value>(*value); }
    }

    const Value& getValue() const
    {
        assert(hasValue());
        if (std::holds_alternative<Value*>(*value)) { return *std::get<Value*>(*value); }
        else { return std::get<Value>(*value); }
    }

    bool isReference() const
    {
        return hasValue() && std::holds_alternative<Value*>(*value);
    }

    bool isDone() const { return status == Status::Done; }
    bool isPending() const { return status == Status::Pending; }
};

// TODO: Generalize these to not return a VisitResult, but anything?
// May be necessary if we create other visitors, but with one visitor
// (the interpreter), maybe this is ok.

namespace ast
{
    class ASTVisitor;

    class ASTNode
    {
        public:
            virtual VisitResult accept(ASTVisitor& visitor) = 0;
            virtual ~ASTNode() = default;
    };

    // Expressions usually evaluate to a Value, but can also be LHS
    // in assignments
    class Expression : public ASTNode {};

    class Constant : public Expression
    {
        public:
            Constant(Value value) : value(value) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Value getValue() const noexcept { return value; };

        private:
            Value value;
    };

    class Variable : public Expression
    {
        public:
            Variable(Name name) : name(name) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Name getName() const noexcept { return name; };

        private:
            Name name;
    };

    class Attribute : public Expression
    {
        public:
            Attribute(std::unique_ptr<Expression> base, String attr)
            : base(std::move(base))
            , attr(attr) {}

            VisitResult accept(ASTVisitor &visitor) override;
            String getAttr() const noexcept { return attr; };
            Expression* getBase() const noexcept { return base.get(); };

        private:
            std::unique_ptr<Expression> base;
            String attr;
    };

    class Assignment : public ASTNode
    {
        public:
            Assignment(std::unique_ptr<Expression> target, std::unique_ptr<Expression> value)
            : target(std::move(target))
            , value(std::move(value)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };
            Expression* getValue() const noexcept { return value.get(); };

        private:
            std::unique_ptr<Expression> target;
            std::unique_ptr<Expression> value;
    };

    class InputTextStatement : public ASTNode
    {
        public:
            InputTextStatement(std::unique_ptr<Variable> player,
                               std::unique_ptr<Expression> target,
                               String prompt)
            : player(std::move(player))
            , target(std::move(target))
            , prompt(prompt) {}

            VisitResult accept(ASTVisitor& visitor) override;
            Variable* getPlayer() const noexcept { return player.get(); }
            Expression* getTarget() const noexcept { return target.get(); }
            String getPrompt() const noexcept { return prompt; }

        private:
            std::unique_ptr<Variable> player;
            std::unique_ptr<Expression> target;
            String prompt;
    };

    class ASTVisitor
    {
        public:
            virtual VisitResult visit(const ASTNode& node) = 0;
            virtual VisitResult visit(const Constant& constant) = 0;
            virtual VisitResult visit(const Variable& variable) = 0;
            virtual VisitResult visit(const Attribute& attribute) = 0;
            virtual VisitResult visit(const Assignment& assignment) = 0;
            virtual VisitResult visit(const InputTextStatement& inputTextStatement) = 0;
    };

    std::unique_ptr<ast::Variable>
    makeVariable(Name name);

    std::unique_ptr<ast::Attribute>
    makeAttribute(std::unique_ptr<ast::Expression> expr, String attr);

    std::unique_ptr<ast::Constant>
    makeConstant(Value value);

    std::unique_ptr<ast::Assignment>
    makeAssignment(std::unique_ptr<ast::Expression> targetExpr,
                std::unique_ptr<ast::Expression> valueToAssign);

    std::unique_ptr<ast::InputTextStatement>
    makeInputTextStmt(std::unique_ptr<ast::Variable> playerVar,
                      std::unique_ptr<ast::Expression> targetExpr,
                      String prompt);

    std::unique_ptr<ast::Constant>
    cloneConstant(ast::Constant* constant);

    std::unique_ptr<ast::Variable>
    cloneVariable(ast::Variable* variable);

    std::unique_ptr<ast::Attribute>
    cloneAttribute(ast::Attribute* attribute);

    std::unique_ptr<ast::Expression>
    cloneExpression(ast::Expression* expr);

    ast::Constant*
    castExpressionToConstant(ast::Expression* expr);

    ast::Variable*
    castExpressionToVariable(ast::Expression* expr);

    ast::Attribute*
    castExpressionToAttribute(ast::Expression* expr);
};
