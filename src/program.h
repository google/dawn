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

#ifndef SRC_PROGRAM_H_
#define SRC_PROGRAM_H_

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/function.h"
#include "src/ast/module.h"
#include "src/ast/variable.h"
#include "src/block_allocator.h"
#include "src/symbol_table.h"
#include "src/traits.h"
#include "src/type/alias_type.h"
#include "src/type/type_manager.h"

namespace tint {

/// Represents all the source in a given program.
class Program {
 public:
  /// ASTNodes is an alias to BlockAllocator<ast::Node>
  using ASTNodes = BlockAllocator<ast::Node>;

  /// Constructor
  Program();

  /// Move constructor
  /// @param rhs the Program to move
  Program(Program&& rhs);

  /// Move assignment operator
  /// @param rhs the Program to move
  /// @return this Program
  Program& operator=(Program&& rhs);

  /// Destructor
  ~Program();

  /// @returns a reference to the program's types
  const type::Manager& Types() const { return types_; }

  /// @returns a reference to the program's AST nodes storage
  const ASTNodes& Nodes() const { return nodes_; }

  /// @returns a reference to the program's AST root Module
  const ast::Module& AST() const { return *ast_; }

  /// @returns a reference to the program's AST root Module
  ast::Module& AST() { return *ast_; }

  /// @returns a reference to the program's SymbolTable
  const SymbolTable& Symbols() const { return symbols_; }

  /// @returns a reference to the program's SymbolTable
  SymbolTable& Symbols() { return symbols_; }

  /// @return a deep copy of this program
  Program Clone() const;

  /// Clone this program into `ctx->dst` using the provided CloneContext
  /// The program will be cloned in this order:
  /// * Constructed types
  /// * Global variables
  /// * Functions
  /// @param ctx the clone context
  void Clone(CloneContext* ctx) const;

  /// @returns true if all required fields in the AST are present.
  bool IsValid() const;

  /// @returns a string representation of the program
  std::string to_str() const;

  /// Creates a new Node owned by the Program. When the Program is
  /// destructed, the Node will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  traits::EnableIfIsType<T, ast::Node>* create(ARGS&&... args) {
    return nodes_.Create<T>(std::forward<ARGS>(args)...);
  }

  /// Creates a new type::Type owned by the Program.
  /// When the Program is destructed, owned Program and the returned
  /// `Type` will also be destructed.
  /// Types are unique (de-aliased), and so calling create() for the same `T`
  /// and arguments will return the same pointer.
  /// @warning Use this method to acquire a type only if all of its type
  /// information is provided in the constructor arguments `args`.<br>
  /// If the type requires additional configuration after construction that
  /// affect its fundamental type, build the type with `std::make_unique`, make
  /// any necessary alterations and then call unique_type() instead.
  /// @param args the arguments to pass to the type constructor
  /// @returns the de-aliased type pointer
  template <typename T, typename... ARGS>
  traits::EnableIfIsType<T, type::Type>* create(ARGS&&... args) {
    static_assert(std::is_base_of<type::Type, T>::value,
                  "T does not derive from type::Type");
    return types_.Get<T>(std::forward<ARGS>(args)...);
  }

  /// Registers `name` as a symbol
  /// @param name the name to register
  /// @returns the symbol for the `name`. If `name` is already registered the
  /// previously generated symbol will be returned.
  /// [DEPRECATED]: Use Symbols().Register()
  Symbol RegisterSymbol(const std::string& name);

  /// Returns the symbol for `name`
  /// @param name the name to lookup
  /// @returns the symbol for name or symbol::kInvalid
  /// [DEPRECATED]: Use Symbols().Get()
  Symbol GetSymbol(const std::string& name) const;

  /// Returns the `name` for `sym`
  /// @param sym the symbol to retrieve the name for
  /// @returns the use provided `name` for the symbol or "" if not found
  /// [DEPRECATED]: Use Symbols().NameFor()
  std::string SymbolToName(const Symbol sym) const;

 private:
  Program(const Program&) = delete;

  type::Manager types_;
  ASTNodes nodes_;
  ast::Module* ast_;
  SymbolTable symbols_;
};

}  // namespace tint

#endif  // SRC_PROGRAM_H_
