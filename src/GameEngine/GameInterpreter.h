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
         * @param constant The Constant to visit.
         * @return VisitResult Containing the value of the Constant as a copy.
         */
        VisitResult visit(const ast::Constant& constant) override;

        /**
         * @brief Evaluates a variable.
         *
         * @param variable The Variable to visit.
         * @return VisitResult Containing the value of the variable as a reference.
         *
         * @pre The variable exists in the Variable Map.
         */
        VisitResult visit(const ast::Variable& variable) override;

        /**
         * @brief Evaluates an attribute.
         *
         * @param attribute The Attribute to visit.
         * @return VisitResult Containing the value of the attribute as a reference.
         *
         * @pre Any base attributes and variables exist in the Variable Map.
         */
        VisitResult visit(const ast::Attribute& attribute) override;

        /**
         * @brief Assigns an expression to a variable or attribute.
         *
         * @param assignment The Assignment to visit.
         * @return VisitResult
         *
         * @pre Any attributes and variables referenced in the expression exist in the
         *      Variable Map.
         * @post A copy of the value produced by expression evaluation is assigned to
         *       the target variable or attribute.
         */
        VisitResult visit(const ast::Assignment& assignment) override;

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

        void setInGameMessages(const std::vector<GameMessage>& inGameMessages);
        std::vector<GameMessage> consumeOutGameMessages();

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

    private:
        VariableMap m_variableMap;

        std::vector<GameMessage> m_inGameMessages;
        std::vector<GameMessage> m_outGameMessages;
};
