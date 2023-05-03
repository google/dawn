// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_PROGRAM_H_
#define SRC_TINT_PROGRAM_H_

#include <string>
#include <unordered_set>

#include "src/tint/ast/function.h"
#include "src/tint/constant/value.h"
#include "src/tint/program_id.h"
#include "src/tint/sem/info.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/manager.h"

// Forward Declarations
namespace tint {
class CloneContext;
}  // namespace tint
namespace tint::ast {
class Module;
}  // namespace tint::ast

namespace tint {

/// Program holds the AST, Type information and SymbolTable for a tint program.
class Program {
  public:
    /// ASTNodeAllocator is an alias to BlockAllocator<ast::Node>
    using ASTNodeAllocator = utils::BlockAllocator<ast::Node>;

    /// SemNodeAllocator is an alias to BlockAllocator<sem::Node>
    using SemNodeAllocator = utils::BlockAllocator<sem::Node>;

    /// ConstantAllocator is an alias to BlockAllocator<constant::Value>
    using ConstantAllocator = utils::BlockAllocator<constant::Value>;

    /// Constructor
    Program();

    /// Move constructor
    /// @param rhs the Program to move
    Program(Program&& rhs);

    /// Move constructor from builder
    /// @param builder the builder used to construct the program
    explicit Program(ProgramBuilder&& builder);

    /// Destructor
    ~Program();

    /// Move assignment operator
    /// @param rhs the Program to move
    /// @return this Program
    Program& operator=(Program&& rhs);

    /// @returns the unique identifier for this program
    ProgramID ID() const { return id_; }

    /// @returns the last allocated (numerically highest) AST node identifier.
    ast::NodeID HighestASTNodeID() const { return highest_node_id_; }

    /// @returns a reference to the program's types
    const type::Manager& Types() const {
        AssertNotMoved();
        return types_;
    }

    /// @returns a reference to the program's AST nodes storage
    const ASTNodeAllocator& ASTNodes() const {
        AssertNotMoved();
        return ast_nodes_;
    }

    /// @returns a reference to the program's semantic nodes storage
    const SemNodeAllocator& SemNodes() const {
        AssertNotMoved();
        return sem_nodes_;
    }

    /// @returns a reference to the program's AST root Module
    const ast::Module& AST() const {
        AssertNotMoved();
        return *ast_;
    }

    /// @returns a reference to the program's semantic info
    const sem::Info& Sem() const {
        AssertNotMoved();
        return sem_;
    }

    /// @returns a reference to the program's SymbolTable
    const SymbolTable& Symbols() const {
        AssertNotMoved();
        return symbols_;
    }

    /// @returns a reference to the program's diagnostics
    const diag::List& Diagnostics() const {
        AssertNotMoved();
        return diagnostics_;
    }

    /// Performs a deep clone of this program.
    /// The returned Program will contain no pointers to objects owned by this
    /// Program, and so after calling, this Program can be safely destructed.
    /// @return a new Program copied from this Program
    Program Clone() const;

    /// Performs a deep clone of this Program's AST nodes, types and symbols into
    /// a new ProgramBuilder. Semantic nodes are not cloned, as these will be
    /// rebuilt when the ProgramBuilder builds its Program.
    /// The returned ProgramBuilder will contain no pointers to objects owned by
    /// this Program, and so after calling, this Program can be safely destructed.
    /// @return a new ProgramBuilder copied from this Program
    ProgramBuilder CloneAsBuilder() const;

    /// @returns true if the program has no error diagnostics and is not missing
    /// information
    bool IsValid() const;

    /// Helper for returning the resolved semantic type of the expression `expr`.
    /// @param expr the AST expression
    /// @return the resolved semantic type for the expression, or nullptr if the
    /// expression has no resolved type.
    const type::Type* TypeOf(const ast::Expression* expr) const;

    /// Helper for returning the resolved semantic type of the variable `var`.
    /// @param var the AST variable
    /// @return the resolved semantic type for the variable, or nullptr if the
    /// variable has no resolved type.
    const type::Type* TypeOf(const ast::Variable* var) const;

    /// Helper for returning the resolved semantic type of the AST type
    /// declaration `type_decl`.
    /// @param type_decl the AST type declaration
    /// @return the resolved semantic type for the type declaration, or nullptr if
    /// the type declaration has no resolved type.
    const type::Type* TypeOf(const ast::TypeDecl* type_decl) const;

    /// A function that can be used to print a program
    using Printer = std::string (*)(const Program*);

    /// The Program printer used for testing and debugging.
    static Printer printer;

  private:
    Program(const Program&) = delete;

    /// Asserts that the program has not been moved.
    void AssertNotMoved() const;

    ProgramID id_;
    ast::NodeID highest_node_id_;
    type::Manager types_;
    ASTNodeAllocator ast_nodes_;
    SemNodeAllocator sem_nodes_;
    ConstantAllocator constant_nodes_;
    ast::Module* ast_ = nullptr;
    sem::Info sem_;
    SymbolTable symbols_{id_};
    diag::List diagnostics_;
    bool is_valid_ = false;  // Not valid until it is built
    bool moved_ = false;
};

/// @param program the Program
/// @returns the ProgramID of the Program
inline ProgramID ProgramIDOf(const Program* program) {
    return program->ID();
}

}  // namespace tint

#endif  // SRC_TINT_PROGRAM_H_
