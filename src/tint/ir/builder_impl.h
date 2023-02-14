// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_IR_BUILDER_IMPL_H_
#define SRC_TINT_IR_BUILDER_IMPL_H_

#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/ast/type.h"
#include "src/tint/constant/clone_context.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/value.h"
#include "src/tint/utils/result.h"

// Forward Declarations
namespace tint {
class Program;
}  // namespace tint
namespace tint::ast {
class Attribute;
class BinaryExpression;
class BitcastExpression;
class BlockStatement;
class BreakIfStatement;
class BreakStatement;
class ContinueStatement;
class Expression;
class ForLoopStatement;
class Function;
class IfStatement;
class LoopStatement;
class LiteralExpression;
class Node;
class ReturnStatement;
class Statement;
class SwitchStatement;
class WhileStatement;
class Variable;
}  // namespace tint::ast
namespace tint::ir {
class Block;
class If;
class Function;
class Loop;
class Switch;
class Terminator;
}  // namespace tint::ir

namespace tint::ir {

/// Builds an ir::Module from a given ast::Program
class BuilderImpl {
  public:
    /// Constructor
    /// @param program the program to create from
    explicit BuilderImpl(const Program* program);
    /// Destructor
    ~BuilderImpl();

    /// Builds an ir::Module from the given Program
    /// @returns true on success, false otherwise
    utils::Result<Module> Build();

    /// @returns the error
    std::string error() const { return diagnostics_.str(); }

    /// Emits a function to the IR.
    /// @param func the function to emit
    /// @returns true if successful, false otherwise
    bool EmitFunction(const ast::Function* func);

    /// Emits a set of statements to the IR.
    /// @param stmts the statements to emit
    /// @returns true if successful, false otherwise.
    bool EmitStatements(utils::VectorRef<const ast::Statement*> stmts);

    /// Emits a statement to the IR
    /// @param stmt the statment to emit
    /// @returns true on success, false otherwise.
    bool EmitStatement(const ast::Statement* stmt);

    /// Emits a block statement to the IR.
    /// @param block the block to emit
    /// @returns true if successful, false otherwise.
    bool EmitBlock(const ast::BlockStatement* block);

    /// Emits an if control node to the IR.
    /// @param stmt the if statement
    /// @returns true if successful, false otherwise.
    bool EmitIf(const ast::IfStatement* stmt);

    /// Emits a return node to the IR.
    /// @param stmt the return AST statement
    /// @returns true if successful, false otherwise.
    bool EmitReturn(const ast::ReturnStatement* stmt);

    /// Emits a loop control node to the IR.
    /// @param stmt the loop statement
    /// @returns true if successful, false otherwise.
    bool EmitLoop(const ast::LoopStatement* stmt);

    /// Emits a loop control node to the IR.
    /// @param stmt the while statement
    /// @returns true if successful, false otherwise.
    bool EmitWhile(const ast::WhileStatement* stmt);

    /// Emits a loop control node to the IR.
    /// @param stmt the for loop statement
    /// @returns true if successful, false otherwise.
    bool EmitForLoop(const ast::ForLoopStatement* stmt);

    /// Emits a switch statement
    /// @param stmt the switch statement
    /// @returns true if successful, false otherwise.
    bool EmitSwitch(const ast::SwitchStatement* stmt);

    /// Emits a break statement
    /// @param stmt the break statement
    /// @returns true if successful, false otherwise.
    bool EmitBreak(const ast::BreakStatement* stmt);

    /// Emits a continue statement
    /// @param stmt the continue statement
    /// @returns true if successful, false otherwise.
    bool EmitContinue(const ast::ContinueStatement* stmt);

    /// Emits a break-if statement
    /// @param stmt the break-if statement
    /// @returns true if successful, false otherwise.
    bool EmitBreakIf(const ast::BreakIfStatement* stmt);

    /// Emits an expression
    /// @param expr the expression to emit
    /// @returns true if successful, false otherwise
    utils::Result<Value*> EmitExpression(const ast::Expression* expr);

    /// Emits a variable
    /// @param var the variable to emit
    /// @returns true if successful, false otherwise
    bool EmitVariable(const ast::Variable* var);

    /// Emits a binary expression
    /// @param expr the binary expression
    /// @returns the value storing the result if successful, utils::Failure otherwise
    utils::Result<Value*> EmitBinary(const ast::BinaryExpression* expr);

    /// Emits a bitcast expression
    /// @param expr the bitcast expression
    /// @returns the value storing the result if successful, utils::Failure otherwise
    utils::Result<Value*> EmitBitcast(const ast::BitcastExpression* expr);

    /// Emits a literal expression
    /// @param lit the literal to emit
    /// @returns true if successful, false otherwise
    utils::Result<Value*> EmitLiteral(const ast::LiteralExpression* lit);

    /// Emits a set of attributes
    /// @param attrs the attributes to emit
    /// @returns true if successful, false otherwise
    bool EmitAttributes(utils::VectorRef<const ast::Attribute*> attrs);

    /// Emits an attribute
    /// @param attr the attribute to emit
    /// @returns true if successful, false otherwise
    bool EmitAttribute(const ast::Attribute* attr);

    /// Retrieve the IR Flow node for a given AST node.
    /// @param n the node to lookup
    /// @returns the FlowNode for the given ast::Node or nullptr if it doesn't exist.
    const ir::FlowNode* FlowNodeForAstNode(const ast::Node* n) const {
        if (ast_to_flow_.count(n) == 0) {
            return nullptr;
        }
        return ast_to_flow_.at(n);
    }

    /// The stack of flow control blocks.
    utils::Vector<FlowNode*, 8> flow_stack;

    /// The IR builder being used by the impl.
    Builder builder;

    /// The current flow block for expressions
    Block* current_flow_block = nullptr;

  private:
    enum class ControlFlags { kNone, kExcludeSwitch };

    void BranchTo(ir::FlowNode* node, utils::VectorRef<Value*> args = {});
    void BranchToIfNeeded(ir::FlowNode* node);

    FlowNode* FindEnclosingControl(ControlFlags flags);

    const Program* program_ = nullptr;

    Symbol CloneSymbol(Symbol sym) const;

    diag::List diagnostics_;

    Function* current_function_ = nullptr;

    /// Map from ast nodes to flow nodes, used to retrieve the flow node for a given AST node.
    /// Used for testing purposes.
    std::unordered_map<const ast::Node*, const FlowNode*> ast_to_flow_;

    constant::CloneContext clone_ctx_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BUILDER_IMPL_H_
