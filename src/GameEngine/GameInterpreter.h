#pragma once

#include <vector>

#include "Types.h"
#include "VariableMap.h"
#include "InputManager.h"
#include "GameMessage.h"
#include "Rules.h"


struct ProgramRaw
{
    std::vector<ast::Statement*> statements;
};


struct Program
{
    std::vector<std::unique_ptr<ast::Statement>> statements;

    ProgramRaw raw()
    {
        std::vector<ast::Statement*> statementsRaw;
        for (auto &statement : statements)
        {
            statementsRaw.push_back(statement.get());
        }
        ProgramRaw programRaw{statementsRaw};
        return programRaw;
    }
};


/**
 * Keeps track of the current statement of a Program.
 *
 * It also allows a context object to be set for the current statement.
 * This allows statements that need to run a program to keep track of
 * their own program iterator.
 *
 * Why an iterator? In short, we need a way to pause and resume execution.
 * A statement may block if it needs user input. In this case, the program
 * must pause execution to allow the caller to handle getting input
 * (e.g., via a networking call). Once the input is retrieved and passed
 * into the interpreter's input manager, the program may resume execution
 * at the statement it left off, which the iterator tracks.
 */
class ProgramIterator
{
    public:
        struct MatchExecutionContext
        {
            std::unique_ptr<ProgramIterator> iterator;
        };

        using StatementContext = std::variant<std::monostate, MatchExecutionContext>;

    public:
        ProgramIterator(ProgramRaw program) : m_statementIndex(0)
        {
            if (program.statements.empty())
            {
                throw std::runtime_error(
                    "Can't create iterator: program must have at least one statement"
                );
            }
            m_program = program;
        }

        void setCurrentContext(StatementContext context)
        {
            if (!currentStatement())
            {
                throw std::runtime_error("Can't set context: no current statement");
            }
            m_context = std::move(context);
        }

        ast::Statement* currentStatement()
        {
            if (m_statementIndex < m_program.statements.size())
            {
                return m_program.statements[m_statementIndex];
            }
            return nullptr;
        }

        template <typename ContextType>
        std::optional<ContextType*> currentContext()
        {
            if (!currentStatement())
            {
                return std::nullopt;
            }
            if (std::holds_alternative<ContextType>(m_context))
            {
                return &std::get<ContextType>(m_context);
            }
            else
            {
                return std::nullopt;
            }
        }

        void goNext()
        {
            if (!currentStatement())
            {
                throw std::runtime_error(
                    "Can't go next: iterator is past the end of the program"
                );
            }
            m_statementIndex++;
            m_context = {};
        }

    private:
        ProgramRaw m_program;
        size_t m_statementIndex;
        StatementContext m_context;
};


class GameInterpreter : public ast::ASTVisitor
{
    public:
        GameInterpreter(InputManager& inputManager, std::optional<Program> program)
            : m_inputManager(inputManager)
            , m_program(std::move(program))
            , m_currentIterator(nullptr)
        {
            if (m_program.has_value())
            {
                m_iterator = std::make_unique<ProgramIterator>(m_program.value().raw());
            }
        }

        void setVariable(const String& name, Value value);

        VisitResult visit(const ast::ASTNode& node) override;

        /**
         * @brief Evaluates a constant.
         *
         * @param constant The Constant node to visit.
         * @return VisitResult Containing the value of the Constant as a copy.
         */
        VisitResult visit(const ast::Constant& constant) override;

        /**
         * @brief Evaluates a variable.
         *
         * @param variable The Variable node to visit.
         * @return VisitResult Containing the value of the variable as a reference.
         *
         * @pre The variable exists in the Variable Map.
         */
        VisitResult visit(const ast::Variable& variable) override;

        /**
         * @brief Evaluates an attribute.
         *
         * @param attribute The Attribute node to visit.
         * @return VisitResult Containing the value of the attribute as a reference.
         *
         * @pre Base expression resolves to a value that has the attribute.
         */
        VisitResult visit(const ast::Attribute& attribute) override;

        /**
         * @brief Compares two values and returns a Boolean.
         *
         * @param comparison The Comparison node to visit.
         * @return VisitResult Containing a Boolean
         *
         * @pre Values are comparable.
         */
        VisitResult visit(const ast::Comparison& comparison) override;

        /**
         * @brief Does a logical operation (e.g., OR) on two values.
         *
         * @param logicalOp The LogicalOperation node to visit.
         * @return VisitResult Containing a Boolean
         *
         * @pre Left/right expressions evaluate to a Boolean.
         */
        VisitResult visit(const ast::LogicalOperation& logicalOp) override;

        /**
         * @brief Does a unary operation (e.g., NOT) on a value.
         *
         * @param unaryOp The UnaryOperation node to visit.
         * @return VisitResult Containing a Boolean
         *
         * @pre Target expression evaluates to a Boolean.
         */
        VisitResult visit(const ast::UnaryOperation& unaryOp) override;

        /**
         * @brief Assigns an evaluated expression to a target.
         *
         * @param assignment The Assignment node to visit.
         * @return VisitResult
         *
         * @pre `target` resolves to a reference.
         * @post A copy of the evaluated expression is assigned to the target.
         */
        VisitResult visit(const ast::Assignment& assignment) override;

        /**
         * @brief Extends a `target` list with elements of the `value` list.
         *
         * @param extend The Extend node to visit.
         * @return VisitResult
         *
         * @pre `target` resolves to a List.
         * @pre `value` evaluates to a List type.
         * @post The elements of the `value` list are appended to the `target` list.
         */
        VisitResult visit(const ast::Extend& extend) override;

        /**
         * @brief Reverses the elements of the `target` list.
         *
         * @param reverse The Reverse node to visit.
         * @return VisitResult
         *
         * @pre `target` resolves to a List.
         * @post The elements of the `target` list are reversed.
         */
        VisitResult visit(const ast::Reverse& reverse) override;

        /**
         * @brief Randomly shuffles the elements of the `target` list.
         *
         * @param shuffle The Shuffle node to visit.
         * @return VisitResult
         *
         * @pre `target` resolves to a List.
         * @post The elements of the `target` list are randomly shuffled.
         */
        VisitResult visit(const ast::Shuffle& shuffle) override;

        /**
         * @brief Discards `amount` elements from the `target` list.
         *
         * @param discard The Discard node to visit.
         * @return VisitResult
         *
         * @pre `target` resolves to a List.
         * @pre `amount` resolves to an Integer.
         * @post At most `amount` items from the `target` list items are removed.
         */
        VisitResult visit(const ast::Discard& discard) override;

        /**
         * @brief Sorts the `target` list in ascending order.
         *
         * @param sort The Sort node to visit.
         * @return VisitResult
         *
         * @pre `target` resolves to a List.
         * @pre The Values of the List are comparable.
         * @pre If a key is provided, the Values of the List are Maps,
         *      the key exists in all Map elements, and the Values at
         *      the key are comparable.
         * @post The elements of the `target` list are sorted.
         */
        VisitResult visit(const ast::Sort& sort) override;

        /**
         * @brief In order, checks if the `target` expression matches the candidate expression.
         * If there's a match, all statements for that candidate are executed in order.
         * Breaks on first match.
         *
         * @param match The Match node to visit.
         * @return VisitResult
         */
        VisitResult visit(const ast::Match& match) override;

        /**
         * @brief Prompts and stores player text input.
         */
        VisitResult visit(const ast::InputText& inputText) override;
        /**
         * @brief Prompts and stores player choice input.
         */
        VisitResult visit(const ast::InputChoice& inputChoice) override;
        /**
         * @brief Prompts and stores player range input.
         */
        VisitResult visit(const ast::InputRange& inputRange) override;
        /**
         * @brief Prompts and stores player vote input.
         */
        VisitResult visit(const ast::InputVote& inputVote) override;

        void execute();

        bool needsIO() const;

        bool isDone() const;

    private:
        Value
        getPlayerAttribute(const ast::Variable& playerVar, String attr);

        void
        doVariableAssignment(ast::Variable& varTarget, Value valueToAssign);

        void
        doAttributeAssignment(ast::Attribute& attrTarget, Value valueToAssign);

        VisitResult
        evaluateExpression(ast::Expression& expr);

        VisitResult
        resolveExpression(ast::Expression& expr);

        Boolean
        isEqual(const Value& a, const Value& b);

        Boolean
        isLessThan(const Value& left, const Value& right);

        std::optional<ast::Match::CandidateRaw>
        findMatch(const ast::Match& match);

        void executeProgram(ProgramIterator& iterator);

    private:
        VariableMap m_variableMap;
        InputManager& m_inputManager;

        std::optional<Program> m_program;
        std::unique_ptr<ProgramIterator> m_iterator;
        ProgramIterator* m_currentIterator;
        bool m_waitingForInput = false;
};
