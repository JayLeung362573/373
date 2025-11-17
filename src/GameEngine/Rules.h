#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <map>

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

    // Statements don't evaluate to a value
    class Statement : public ASTNode {};

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

    class Comparison : public Expression
    {
        public:
            enum class Kind { LT, EQ };

            Comparison(std::unique_ptr<Expression> left,
                       std::unique_ptr<Expression> right,
                       Kind kind)
            : left(std::move(left))
            , right(std::move(right))
            , kind(kind) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getLeft() const noexcept { return left.get(); };
            Expression* getRight() const noexcept { return right.get(); };
            Kind getKind() const noexcept { return kind; };

        private:
            std::unique_ptr<Expression> left;
            std::unique_ptr<Expression> right;

            Kind kind;
    };

    class LogicalOperation : public Expression
    {
        public:
            enum class Kind { OR };

            LogicalOperation(std::unique_ptr<Expression> left,
                             std::unique_ptr<Expression> right,
                             Kind kind)
            : left(std::move(left))
            , right(std::move(right))
            , kind(kind) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getLeft() const noexcept { return left.get(); };
            Expression* getRight() const noexcept { return right.get(); };
            Kind getKind() const noexcept { return kind; };

        private:
            std::unique_ptr<Expression> left;
            std::unique_ptr<Expression> right;

            Kind kind;
    };

    class UnaryOperation : public Expression
    {
        public:
            enum class Kind { NOT };

            UnaryOperation(std::unique_ptr<Expression> target, Kind kind)
            : target(std::move(target))
            , kind(kind) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };
            Kind getKind() const noexcept { return kind; };

        private:
            std::unique_ptr<Expression> target;
            Kind kind;
    };

    class Assignment : public Statement
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

    class Extend : public Statement
    {
        public:
            Extend(std::unique_ptr<Expression> target, std::unique_ptr<Expression> value)
            : target(std::move(target))
            , value(std::move(value)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };
            Expression* getValue() const noexcept { return value.get(); };

        private:
            std::unique_ptr<Expression> target;
            std::unique_ptr<Expression> value;
    };

    class Reverse : public Statement
    {
        public:
            Reverse(std::unique_ptr<Expression> target) : target(std::move(target)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };

        private:
            std::unique_ptr<Expression> target;
    };

    class Shuffle : public Statement
    {
        public:
            Shuffle(std::unique_ptr<Expression> target) : target(std::move(target)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };

        private:
            std::unique_ptr<Expression> target;
    };

    class Discard : public Statement
    {
        public:
            Discard(std::unique_ptr<Expression> target, std::unique_ptr<Expression> amount)
            : target(std::move(target))
            , amount(std::move(amount)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };
            Expression* getAmount() const noexcept { return amount.get(); };

        private:
            std::unique_ptr<Expression> target;
            std::unique_ptr<Expression> amount;
    };

    class Sort : public Statement
    {
        public:
            Sort(std::unique_ptr<Expression> target, std::optional<String> key = {})
            : target(std::move(target))
            , key(key) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };
            std::optional<String> getKey() const noexcept { return key; }

        private:
            std::unique_ptr<Expression> target;
            std::optional<String> key;
    };

    class Match : public Statement
    {
        public:
            struct ExpressionCandidateAndStatementsPair
            {
                std::unique_ptr<Expression> expressionCandidate;
                std::vector<std::unique_ptr<Statement>> statements;
            };

            struct ExpressionCandidateAndStatementsPairRaw
            {
                Expression* expressionCandidate;
                std::vector<Statement*> statements;
            };

            Match(std::unique_ptr<Expression> target,
                  std::vector<ExpressionCandidateAndStatementsPair> pairs)
            : target(std::move(target))
            , pairs(std::move(pairs)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };

            std::vector<ExpressionCandidateAndStatementsPairRaw>
            getExpressionCandidateAndStatementsPairs() const
            {
                std::vector<ExpressionCandidateAndStatementsPairRaw> rawPairs;
                for (auto& pair : pairs)
                {
                    Expression* expressionCandidate = pair.expressionCandidate.get();

                    std::vector<Statement*> statements;
                    for (auto& statement : pair.statements)
                    {
                        statements.push_back(statement.get());
                    }
                    rawPairs.push_back({expressionCandidate, statements});
                }
                return rawPairs;
            }

        private:
            std::unique_ptr<Expression> target;
            std::vector<ExpressionCandidateAndStatementsPair> pairs;
    };

    class InputTextStatement : public Statement
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
            virtual VisitResult visit(const Comparison& comparison) = 0;
            virtual VisitResult visit(const LogicalOperation& logicalOp) = 0;
            virtual VisitResult visit(const UnaryOperation& unaryOp) = 0;
            virtual VisitResult visit(const Assignment& assignment) = 0;
            virtual VisitResult visit(const Extend& extend) = 0;
            virtual VisitResult visit(const Reverse& reverse) = 0;
            virtual VisitResult visit(const Shuffle& shuffle) = 0;
            virtual VisitResult visit(const Discard& discard) = 0;
            virtual VisitResult visit(const Sort& sort) = 0;
            virtual VisitResult visit(const Match& match) = 0;
            virtual VisitResult visit(const InputTextStatement& inputTextStatement) = 0;
    };

    std::unique_ptr<ast::Variable>
    makeVariable(Name name);

    std::unique_ptr<ast::Attribute>
    makeAttribute(std::unique_ptr<ast::Expression> expr, String attr);

    std::unique_ptr<ast::Constant>
    makeConstant(Value value);

    std::unique_ptr<ast::Comparison>
    makeComparison(std::unique_ptr<ast::Expression> left,
                   std::unique_ptr<ast::Expression> right,
                   ast::Comparison::Kind kind);

    std::unique_ptr<ast::LogicalOperation>
    makeLogicalOperation(std::unique_ptr<ast::Expression> left,
                         std::unique_ptr<ast::Expression> right,
                         ast::LogicalOperation::Kind kind);

    std::unique_ptr<ast::UnaryOperation>
    makeUnaryOperation(std::unique_ptr<ast::Expression> target,
                       ast::UnaryOperation::Kind kind);

    std::unique_ptr<ast::Assignment>
    makeAssignment(std::unique_ptr<ast::Expression> targetExpr,
                std::unique_ptr<ast::Expression> valueToAssign);

    std::unique_ptr<ast::Extend>
    makeExtend(std::unique_ptr<ast::Expression> target,
               std::unique_ptr<ast::Expression> value);

    std::unique_ptr<ast::Reverse>
    makeReverse(std::unique_ptr<ast::Expression> target);

    std::unique_ptr<ast::Shuffle>
    makeShuffle(std::unique_ptr<ast::Expression> target);

    std::unique_ptr<ast::Discard>
    makeDiscard(std::unique_ptr<ast::Expression> target,
                std::unique_ptr<ast::Expression> amount);

    std::unique_ptr<ast::Sort>
    makeSort(std::unique_ptr<ast::Expression> target,
             std::optional<String> key = {});

    std::unique_ptr<ast::Match>
    makeMatch(std::unique_ptr<ast::Expression> target,
              std::vector<ast::Match::ExpressionCandidateAndStatementsPair> pairs);

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

    // Builder classes allow us to define these types inline, which may make it easier to set up complex trees
    class StatementsBuilder
    {
        public:
            ast::StatementsBuilder& addStatement(std::unique_ptr<ast::Statement> statement)
            {
                statements.push_back(std::move(statement));
                return *this;
            }

            std::vector<std::unique_ptr<ast::Statement>> build()
            {
                return std::move(statements);
            }

        private:
            std::vector<std::unique_ptr<ast::Statement>> statements;
    };

    class MatchBuilder
    {
        public:
            ast::MatchBuilder& setTarget(std::unique_ptr<ast::Expression> target)
            {
                m_target = std::move(target);
                return *this;
            }

            ast::MatchBuilder& addCandidatePair(ast::Match::ExpressionCandidateAndStatementsPair pair)
            {
                m_pairs.push_back(std::move(std::move(pair)));
                return *this;
            }

            std::unique_ptr<ast::Match> build()
            {
                return ast::makeMatch(std::move(m_target), std::move(m_pairs));
            }

        private:
            std::unique_ptr<ast::Expression> m_target;
            std::vector<ast::Match::ExpressionCandidateAndStatementsPair> m_pairs;
    };

    struct GameRules
    {
        std::vector<std::unique_ptr<ast::Statement>> statements;
    };
};
