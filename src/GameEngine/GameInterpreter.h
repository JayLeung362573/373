#pragma once

#include <vector>

#include "Types.h"
#include "VariableMap.h"
#include "GameMessage.h"
#include "Rules.h"

class GameInterpreter : public ast::ASTVisitor
{
    public:
        GameInterpreter() = default;

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
         * @brief Prompts and stores player text input.
         *
         * This method checks for player input in inGameMessages. If there's no input,
         * it pushes a message to outGameMessages to prompt the player. If input is
         * found, it assigns the input to the target variable or attribute.
         *
         * Note: This setup with the in/out messages may not be very practical??
         *       Might add complexity if we need to handle not sending duplicate messages.
         *       Maybe we need some kind of InputManager?
         *
         * @param inputTextStatement The InputTextStatement to visit.
         * @return VisitResult Status of the input processing (Done or Pending).
         *
         * @pre The player variable exists in the Variable Map and has an "id" attribute.
         * @post The player's input is assigned to the target variable or attribute.
         */
        VisitResult visit(const ast::InputTextStatement& inputTextStatement) override;

        /**
         * @brief Evaluates a match statement (switch-case style control flow).
         *
         * @param match The Match node to visit.
         * @return VisitResult
         *
         * @pre The target expression can be evaluated.
         * @post The first matching case's statements are executed.
         */
        VisitResult visit(const ast::Match& match) override;

        void setInGameMessages(const std::vector<GameMessage>& inGameMessages);
        std::vector<GameMessage> consumeOutGameMessages();

        /**
         * @brief Get read-only access to the game state (variable map).
         * Useful for inspecting game state after execution.
         */
        const VariableMap& getGameState() const { return m_variableMap; }

    private:
        std::optional<TextInputMessage>
        getTextInputMsg(String playerID, String prompt) const;

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

    private:
        VariableMap m_variableMap;

        std::vector<GameMessage> m_inGameMessages;
        std::vector<GameMessage> m_outGameMessages;
};
