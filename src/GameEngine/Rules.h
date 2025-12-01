#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <map>

#include "Types.h"


struct VisitResult
{
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

    class ArithmeticOperation : public Expression
    {
        public:
            enum class Kind { ADD };

            ArithmeticOperation(std::unique_ptr<Expression> left,
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

    class Callable : public Expression
    {
        public:
            enum class Kind { SIZE, UP_FROM };

            Callable(std::unique_ptr<Expression> left,
                     std::vector<std::unique_ptr<Expression>> args,
                     Kind kind)
            : left(std::move(left))
            , args(std::move(args))
            , kind(std::move(kind)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getLeft() const noexcept { return left.get(); };

            std::vector<Expression*> getArgs() const noexcept
            {
                std::vector<Expression*> rawExpressions;
                for (auto& expression : args)
                {
                    rawExpressions.push_back(expression.get());
                }
                return rawExpressions;
            }

            Kind getKind() const noexcept { return kind; };

        private:
            std::unique_ptr<Expression> left;
            std::vector<std::unique_ptr<Expression>> args;

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
            struct Candidate
            {
                std::unique_ptr<Expression> expressionCandidate;
                std::vector<std::unique_ptr<Statement>> statements;
            };

            struct CandidateRaw
            {
                Expression* expressionCandidate;
                std::vector<Statement*> statements;
            };

            Match(std::unique_ptr<Expression> target,
                  std::vector<Candidate> candidates)
            : target(std::move(target))
            , candidates(std::move(candidates)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Expression* getTarget() const noexcept { return target.get(); };

            std::vector<CandidateRaw>
            getCandidates() const
            {
                std::vector<CandidateRaw> rawCandidates;
                for (auto& candidate : candidates)
                {
                    Expression* expressionCandidate = candidate.expressionCandidate.get();

                    std::vector<Statement*> statements;
                    for (auto& statement : candidate.statements)
                    {
                        statements.push_back(statement.get());
                    }
                    rawCandidates.push_back({expressionCandidate, statements});
                }
                return rawCandidates;
            }

        private:
            std::unique_ptr<Expression> target;
            std::vector<Candidate> candidates;
    };

    class ForLoop : public Statement
    {
        public:
            ForLoop(std::unique_ptr<Variable> element,
                        std::unique_ptr<Expression> target,
                        std::vector<std::unique_ptr<Statement>> statements)
            : element(std::move(element))
            , target(std::move(target))
            , statements(std::move(statements)) {}

            VisitResult accept(ASTVisitor &visitor) override;
            Variable* getElement() const noexcept { return element.get(); };
            Expression* getTarget() const noexcept { return target.get(); };

            std::vector<Statement*>
            getStatements() const
            {
                // This logic is also in Match, now duplicating
                // TODO: Have a StatementList type that stores std::unique_ptr<Statement>,
                // with raw() method to get vector of Statement* ?
                std::vector<Statement*> rawStatements;
                for (auto& statement : statements)
                {
                    rawStatements.push_back(statement.get());
                }
                return rawStatements;
            }

        private:
            std::unique_ptr<Variable> element;
            std::unique_ptr<Expression> target;
            std::vector<std::unique_ptr<Statement>> statements;
    };

    class InputText : public Statement
    {
        public:
            InputText(std::unique_ptr<Variable> player,
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

    class InputChoice : public Statement
    {
        public:
            InputChoice(std::unique_ptr<Variable> player,
                        std::unique_ptr<Expression> target,
                        String prompt,
                        std::unique_ptr<Expression> choices)
            : player(std::move(player))
            , target(std::move(target))
            , prompt(prompt)
            , choices(std::move(choices)) {}

            VisitResult accept(ASTVisitor& visitor) override;
            Variable* getPlayer() const noexcept { return player.get(); }
            Expression* getTarget() const noexcept { return target.get(); }
            String getPrompt() const noexcept { return prompt; }
            Expression* getChoices() const noexcept { return choices.get(); }

        private:
            std::unique_ptr<Variable> player;
            std::unique_ptr<Expression> target;
            String prompt;
            std::unique_ptr<Expression> choices;
    };

    class InputRange : public Statement
    {
        public:
            InputRange(std::unique_ptr<Variable> player,
                       std::unique_ptr<Expression> target,
                       String prompt,
                       std::unique_ptr<Expression> minValue,
                       std::unique_ptr<Expression> maxValue)
            : player(std::move(player))
            , target(std::move(target))
            , prompt(prompt)
            , minValue(std::move(minValue))
            , maxValue(std::move(maxValue)) {}

            VisitResult accept(ASTVisitor& visitor) override;
            Variable* getPlayer() const noexcept { return player.get(); }
            Expression* getTarget() const noexcept { return target.get(); }
            String getPrompt() const noexcept { return prompt; }
            Expression* getMinValue() const noexcept { return minValue.get(); }
            Expression* getMaxValue() const noexcept { return maxValue.get(); }

        private:
            std::unique_ptr<Variable> player;
            std::unique_ptr<Expression> target;
            String prompt;
            std::unique_ptr<Expression> minValue;
            std::unique_ptr<Expression> maxValue;
    };

    class InputVote : public Statement
    {
        public:
            InputVote(std::unique_ptr<Variable> player,
                               std::unique_ptr<Expression> target,
                               String prompt,
                               std::unique_ptr<Expression> choices)
            : player(std::move(player))
            , target(std::move(target))
            , prompt(prompt)
            , choices(std::move(choices)) {}

            VisitResult accept(ASTVisitor& visitor) override;
            Variable* getPlayer() const noexcept { return player.get(); }
            Expression* getTarget() const noexcept { return target.get(); }
            String getPrompt() const noexcept { return prompt; }
            Expression* getChoices() const noexcept { return choices.get(); }

        private:
            std::unique_ptr<Variable> player;
            std::unique_ptr<Expression> target;
            String prompt;
            std::unique_ptr<Expression> choices;
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
            virtual VisitResult visit(const ArithmeticOperation& arithmeticOp) = 0;
            virtual VisitResult visit(const Callable& callable) = 0;
            virtual VisitResult visit(const Assignment& assignment) = 0;
            virtual VisitResult visit(const Extend& extend) = 0;
            virtual VisitResult visit(const Reverse& reverse) = 0;
            virtual VisitResult visit(const Shuffle& shuffle) = 0;
            virtual VisitResult visit(const Discard& discard) = 0;
            virtual VisitResult visit(const Sort& sort) = 0;
            virtual VisitResult visit(const Match& match) = 0;
            virtual VisitResult visit(const ForLoop& forLoop) = 0;
            virtual VisitResult visit(const InputText& inputText) = 0;
            virtual VisitResult visit(const InputChoice& inputChoice) = 0;
            virtual VisitResult visit(const InputRange& inputRange) = 0;
            virtual VisitResult visit(const InputVote& inputVote) = 0;
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

    std::unique_ptr<ast::ArithmeticOperation>
    makeArithmeticOperation(std::unique_ptr<ast::Expression> left,
                            std::unique_ptr<ast::Expression> right,
                            ast::ArithmeticOperation::Kind kind);

    std::unique_ptr<ast::Callable>
    makeCallable(std::unique_ptr<ast::Expression> left,
                 std::vector<std::unique_ptr<Expression>> args,
                 ast::Callable::Kind kind);

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
              std::vector<ast::Match::Candidate> candidates);

    std::unique_ptr<ast::ForLoop>
    makeForLoop(std::unique_ptr<Variable> element,
                std::unique_ptr<ast::Expression> target,
                std::vector<std::unique_ptr<ast::Statement>> statements);

    std::unique_ptr<ast::InputText>
    makeInputText(std::unique_ptr<ast::Variable> playerVar,
                  std::unique_ptr<ast::Expression> targetExpr,
                  String prompt);

    std::unique_ptr<ast::InputChoice>
    makeInputChoice(std::unique_ptr<ast::Variable> playerVar,
                    std::unique_ptr<ast::Expression> targetExpr,
                    String prompt,
                    std::unique_ptr<ast::Expression> choices);

    std::unique_ptr<ast::InputRange>
    makeInputRange(std::unique_ptr<ast::Variable> playerVar,
                   std::unique_ptr<ast::Expression> targetExpr,
                   String prompt,
                   std::unique_ptr<ast::Expression> minValue,
                   std::unique_ptr<ast::Expression> maxValue);

    std::unique_ptr<ast::InputVote>
    makeInputVote(std::unique_ptr<ast::Variable> playerVar,
                  std::unique_ptr<ast::Expression> targetExpr,
                  String prompt,
                  std::unique_ptr<ast::Expression> choices);

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

    class ExpressionsBuilder
    {
        public:
            ast::ExpressionsBuilder& addExpression(std::unique_ptr<ast::Expression> expression)
            {
                expressions.push_back(std::move(expression));
                return *this;
            }

            std::vector<std::unique_ptr<ast::Expression>> build()
            {
                return std::move(expressions);
            }

        private:
            std::vector<std::unique_ptr<ast::Expression>> expressions;
    };

    class MatchBuilder
    {
        public:
            ast::MatchBuilder& setTarget(std::unique_ptr<ast::Expression> target)
            {
                m_target = std::move(target);
                return *this;
            }

            ast::MatchBuilder& addCandidatePair(std::unique_ptr<Expression> expressionCandidate,
                                                std::vector<std::unique_ptr<Statement>> statements)
            {
                ast::Match::Candidate pair{
                    std::move(expressionCandidate), std::move(statements)
                };
                m_candidates.push_back(std::move(std::move(pair)));
                return *this;
            }

            std::unique_ptr<ast::Match> build()
            {
                return ast::makeMatch(std::move(m_target), std::move(m_candidates));
            }

        private:
            std::unique_ptr<ast::Expression> m_target;
            std::vector<ast::Match::Candidate> m_candidates;
    };
    struct GameRules
    {
        std::vector<std::unique_ptr<ast::Statement>> statements;
    };
};
