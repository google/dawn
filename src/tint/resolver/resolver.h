// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_RESOLVER_RESOLVER_H_
#define SRC_TINT_RESOLVER_RESOLVER_H_

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/resolver/dependency_graph.h"
#include "src/tint/resolver/intrinsic_table.h"
#include "src/tint/resolver/sem_helper.h"
#include "src/tint/resolver/validator.h"
#include "src/tint/scope_stack.h"
#include "src/tint/sem/binding_point.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/struct.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/unique_vector.h"

// Forward declarations
namespace tint::ast {
class IndexAccessorExpression;
class BinaryExpression;
class BitcastExpression;
class CallExpression;
class CallStatement;
class CaseStatement;
class ForLoopStatement;
class Function;
class IdentifierExpression;
class LoopStatement;
class MemberAccessorExpression;
class ReturnStatement;
class SwitchStatement;
class UnaryOpExpression;
class Variable;
}  // namespace tint::ast
namespace tint::sem {
class Array;
class Atomic;
class BlockStatement;
class Builtin;
class CaseStatement;
class ForLoopStatement;
class IfStatement;
class LoopStatement;
class Statement;
class SwitchStatement;
class TypeConstructor;
}  // namespace tint::sem

namespace tint::resolver {

/// Resolves types for all items in the given tint program
class Resolver {
  public:
    /// Constructor
    /// @param builder the program builder
    explicit Resolver(ProgramBuilder* builder);

    /// Destructor
    ~Resolver();

    /// @returns error messages from the resolver
    std::string error() const { return diagnostics_.str(); }

    /// @returns true if the resolver was successful
    bool Resolve();

    /// @param type the given type
    /// @returns true if the given type is a plain type
    bool IsPlain(const sem::Type* type) const { return validator_.IsPlain(type); }

    /// @param type the given type
    /// @returns true if the given type is a fixed-footprint type
    bool IsFixedFootprint(const sem::Type* type) const { return validator_.IsFixedFootprint(type); }

    /// @param type the given type
    /// @returns true if the given type is storable
    bool IsStorable(const sem::Type* type) const { return validator_.IsStorable(type); }

    /// @param type the given type
    /// @returns true if the given type is host-shareable
    bool IsHostShareable(const sem::Type* type) const { return validator_.IsHostShareable(type); }

    /// @returns the validator for testing
    const Validator* GetValidatorForTesting() const { return &validator_; }

  private:
    /// Describes the context in which a variable is declared
    enum class VariableKind { kParameter, kLocal, kGlobal };

    Validator::ValidTypeStorageLayouts valid_type_storage_layouts_;

    /// Structure holding semantic information about a block (i.e. scope), such as
    /// parent block and variables declared in the block.
    /// Used to validate variable scoping rules.
    struct BlockInfo {
        enum class Type { kGeneric, kLoop, kLoopContinuing, kSwitchCase };

        BlockInfo(const ast::BlockStatement* block, Type type, BlockInfo* parent);
        ~BlockInfo();

        template <typename Pred>
        BlockInfo* FindFirstParent(Pred&& pred) {
            BlockInfo* curr = this;
            while (curr && !pred(curr)) {
                curr = curr->parent;
            }
            return curr;
        }

        BlockInfo* FindFirstParent(BlockInfo::Type ty) {
            return FindFirstParent([ty](auto* block_info) { return block_info->type == ty; });
        }

        ast::BlockStatement const* const block;
        const Type type;
        BlockInfo* const parent;
        std::vector<const ast::Variable*> decls;

        // first_continue is set to the index of the first variable in decls
        // declared after the first continue statement in a loop block, if any.
        constexpr static size_t kNoContinue = size_t(~0);
        size_t first_continue = kNoContinue;
    };

    // Structure holding information for a TypeDecl
    struct TypeDeclInfo {
        ast::TypeDecl const* const ast;
        sem::Type* const sem;
    };

    /// Resolves the program, without creating final the semantic nodes.
    /// @returns true on success, false on error
    bool ResolveInternal();

    /// Creates the nodes and adds them to the sem::Info mappings of the
    /// ProgramBuilder.
    void CreateSemanticNodes() const;

    /// Retrieves information for the requested import.
    /// @param src the source of the import
    /// @param path the import path
    /// @param name the method name to get information on
    /// @param params the parameters to the method call
    /// @param id out parameter for the external call ID. Must not be a nullptr.
    /// @returns the return type of `name` in `path` or nullptr on error.
    sem::Type* GetImportData(const Source& src,
                             const std::string& path,
                             const std::string& name,
                             const ast::ExpressionList& params,
                             uint32_t* id);

    //////////////////////////////////////////////////////////////////////////////
    // AST and Type traversal methods
    //////////////////////////////////////////////////////////////////////////////

    // Expression resolving methods
    // Returns the semantic node pointer on success, nullptr on failure.
    sem::Expression* IndexAccessor(const ast::IndexAccessorExpression*);
    sem::Expression* Binary(const ast::BinaryExpression*);
    sem::Expression* Bitcast(const ast::BitcastExpression*);
    sem::Call* Call(const ast::CallExpression*);
    sem::Expression* Expression(const ast::Expression*);
    sem::Function* Function(const ast::Function*);
    sem::Call* FunctionCall(const ast::CallExpression*,
                            sem::Function* target,
                            std::vector<const sem::Expression*> args,
                            sem::Behaviors arg_behaviors);
    sem::Expression* Identifier(const ast::IdentifierExpression*);
    sem::Call* BuiltinCall(const ast::CallExpression*,
                           sem::BuiltinType,
                           std::vector<const sem::Expression*> args);
    sem::Expression* Literal(const ast::LiteralExpression*);
    sem::Expression* MemberAccessor(const ast::MemberAccessorExpression*);
    sem::Expression* UnaryOp(const ast::UnaryOpExpression*);

    /// If `expr` is not of an abstract-numeric type, then Materialize() will just return `expr`.
    /// If `expr` is of an abstract-numeric type:
    /// * Materialize will create and return a sem::Materialize node wrapping `expr`.
    /// * The AST -> Sem binding will be updated to point to the new sem::Materialize node.
    /// * The sem::Materialize node will have a new concrete type, which will be `target_type` if
    ///   not nullptr, otherwise:
    ///     * a type with the element type of `i32` (e.g. `i32`, `vec2<i32>`) if `expr` has a
    ///       element type of abstract-integer...
    ///     * ... or a type with the element type of `f32` (e.g. `f32`, vec3<f32>`, `mat2x3<f32>`)
    ///       if `expr` has a element type of abstract-float.
    /// * The sem::Materialize constant value will be the value of `expr` value-converted to the
    ///   materialized type.
    /// If `expr` is nullptr, then Materialize() will also return nullptr.
    const sem::Expression* Materialize(const sem::Expression* expr,
                                       const sem::Type* target_type = nullptr);

    /// Materializes all the arguments in `args` to the parameter types of `target`.
    /// @returns true on success, false on failure.
    bool MaterializeArguments(std::vector<const sem::Expression*>& args,
                              const sem::CallTarget* target);

    /// @returns true if an argument of an abstract numeric type, passed to a parameter of type
    /// `parameter_ty` should be materialized.
    bool ShouldMaterializeArgument(const sem::Type* parameter_ty) const;

    // Statement resolving methods
    // Each return true on success, false on failure.
    sem::Statement* AssignmentStatement(const ast::AssignmentStatement*);
    sem::BlockStatement* BlockStatement(const ast::BlockStatement*);
    sem::Statement* BreakStatement(const ast::BreakStatement*);
    sem::Statement* CallStatement(const ast::CallStatement*);
    sem::CaseStatement* CaseStatement(const ast::CaseStatement*);
    sem::Statement* CompoundAssignmentStatement(const ast::CompoundAssignmentStatement*);
    sem::Statement* ContinueStatement(const ast::ContinueStatement*);
    sem::Statement* DiscardStatement(const ast::DiscardStatement*);
    sem::Statement* FallthroughStatement(const ast::FallthroughStatement*);
    sem::ForLoopStatement* ForLoopStatement(const ast::ForLoopStatement*);
    sem::GlobalVariable* GlobalVariable(const ast::Variable*);
    sem::Statement* Parameter(const ast::Variable*);
    sem::IfStatement* IfStatement(const ast::IfStatement*);
    sem::Statement* IncrementDecrementStatement(const ast::IncrementDecrementStatement*);
    sem::LoopStatement* LoopStatement(const ast::LoopStatement*);
    sem::Statement* ReturnStatement(const ast::ReturnStatement*);
    sem::Statement* Statement(const ast::Statement*);
    sem::SwitchStatement* SwitchStatement(const ast::SwitchStatement* s);
    sem::Statement* VariableDeclStatement(const ast::VariableDeclStatement*);
    bool Statements(const ast::StatementList&);

    // CollectTextureSamplerPairs() collects all the texture/sampler pairs from the target function
    // / builtin, and records these on the current function by calling AddTextureSamplerPair().
    void CollectTextureSamplerPairs(sem::Function* func,
                                    const std::vector<const sem::Expression*>& args) const;
    void CollectTextureSamplerPairs(const sem::Builtin* builtin,
                                    const std::vector<const sem::Expression*>& args) const;

    /// Resolves the WorkgroupSize for the given function, assigning it to
    /// current_function_
    bool WorkgroupSize(const ast::Function*);

    /// @returns the sem::Type for the ast::Type `ty`, building it if it
    /// hasn't been constructed already. If an error is raised, nullptr is
    /// returned.
    /// @param ty the ast::Type
    sem::Type* Type(const ast::Type* ty);

    /// @param enable the enable declaration
    /// @returns the resolved extension
    bool Enable(const ast::Enable* enable);

    /// @param named_type the named type to resolve
    /// @returns the resolved semantic type
    sem::Type* TypeDecl(const ast::TypeDecl* named_type);

    /// Builds and returns the semantic information for the array `arr`.
    /// This method does not mark the ast::Array node, nor attach the generated
    /// semantic information to the AST node.
    /// @returns the semantic Array information, or nullptr if an error is
    /// raised.
    /// @param arr the Array to get semantic information for
    sem::Array* Array(const ast::Array* arr);

    /// Builds and returns the semantic information for the alias `alias`.
    /// This method does not mark the ast::Alias node, nor attach the generated
    /// semantic information to the AST node.
    /// @returns the aliased type, or nullptr if an error is raised.
    sem::Type* Alias(const ast::Alias* alias);

    /// Builds and returns the semantic information for the structure `str`.
    /// This method does not mark the ast::Struct node, nor attach the generated
    /// semantic information to the AST node.
    /// @returns the semantic Struct information, or nullptr if an error is
    /// raised.
    sem::Struct* Structure(const ast::Struct* str);

    /// @returns the semantic info for the variable `var`. If an error is
    /// raised, nullptr is returned.
    /// @note this method does not resolve the attributes as these are
    /// context-dependent (global, local, parameter)
    /// @param var the variable to create or return the `VariableInfo` for
    /// @param kind what kind of variable we are declaring
    /// @param index the index of the parameter, if this variable is a parameter
    sem::Variable* Variable(const ast::Variable* var, VariableKind kind, uint32_t index = 0);

    /// Records the storage class usage for the given type, and any transient
    /// dependencies of the type. Validates that the type can be used for the
    /// given storage class, erroring if it cannot.
    /// @param sc the storage class to apply to the type and transitent types
    /// @param ty the type to apply the storage class on
    /// @param usage the Source of the root variable declaration that uses the
    /// given type and storage class. Used for generating sensible error
    /// messages.
    /// @returns true on success, false on error
    bool ApplyStorageClassUsageToType(ast::StorageClass sc, sem::Type* ty, const Source& usage);

    /// @param storage_class the storage class
    /// @returns the default access control for the given storage class
    ast::Access DefaultAccessForStorageClass(ast::StorageClass storage_class);

    /// Allocate constant IDs for pipeline-overridable constants.
    void AllocateOverridableConstantIds();

    /// Set the shadowing information on variable declarations.
    /// @note this method must only be called after all semantic nodes are built.
    void SetShadows();
    /// StatementScope() does the following:
    /// * Creates the AST -> SEM mapping.
    /// * Assigns `sem` to #current_statement_
    /// * Assigns `sem` to #current_compound_statement_ if `sem` derives from
    ///   sem::CompoundStatement.
    /// * Assigns `sem` to #current_block_ if `sem` derives from
    ///   sem::BlockStatement.
    /// * Then calls `callback`.
    /// * Before returning #current_statement_, #current_compound_statement_, and
    ///   #current_block_ are restored to their original values.
    /// @returns `sem` if `callback` returns true, otherwise `nullptr`.
    template <typename SEM, typename F>
    SEM* StatementScope(const ast::Statement* ast, SEM* sem, F&& callback);

    /// Mark records that the given AST node has been visited, and asserts that
    /// the given node has not already been seen. Diamonds in the AST are
    /// illegal.
    /// @param node the AST node.
    /// @returns true on success, false on error
    bool Mark(const ast::Node* node);

    /// Adds the given error message to the diagnostics
    void AddError(const std::string& msg, const Source& source) const;

    /// Adds the given warning message to the diagnostics
    void AddWarning(const std::string& msg, const Source& source) const;

    /// Adds the given note message to the diagnostics
    void AddNote(const std::string& msg, const Source& source) const;

    //////////////////////////////////////////////////////////////////////////////
    /// Constant value evaluation methods
    //////////////////////////////////////////////////////////////////////////////
    /// The result type of a ConstantEvaluation method. Holds the constant value and a boolean,
    /// which is true on success, false on an error.
    using ConstantResult = utils::Result<sem::Constant>;

    /// Convert the `value` to `target_type`
    /// @return the converted value
    ConstantResult ConvertValue(const sem::Constant& value,
                                const sem::Type* target_type,
                                const Source& source);
    ConstantResult EvaluateConstantValue(const ast::Expression* expr, const sem::Type* type);
    ConstantResult EvaluateConstantValue(const ast::LiteralExpression* literal,
                                         const sem::Type* type);
    ConstantResult EvaluateConstantValue(const ast::CallExpression* call, const sem::Type* type);

    /// @returns true if the symbol is the name of a builtin function.
    bool IsBuiltin(Symbol) const;

    // ArrayConstructorSig represents a unique array constructor signature.
    // It is a tuple of the array type and number of arguments provided.
    using ArrayConstructorSig = utils::UnorderedKeyWrapper<std::tuple<const sem::Array*, size_t>>;

    // StructConstructorSig represents a unique structure constructor signature.
    // It is a tuple of the structure type and number of arguments provided.
    using StructConstructorSig = utils::UnorderedKeyWrapper<std::tuple<const sem::Struct*, size_t>>;

    ProgramBuilder* const builder_;
    diag::List& diagnostics_;
    std::unique_ptr<IntrinsicTable> const intrinsic_table_;
    DependencyGraph dependencies_;
    SemHelper sem_;
    Validator validator_;
    ast::Extensions enabled_extensions_;
    std::vector<sem::Function*> entry_points_;
    std::unordered_map<const sem::Type*, const Source&> atomic_composite_info_;
    std::unordered_set<const ast::Node*> marked_;
    std::unordered_map<uint32_t, const sem::Variable*> constant_ids_;
    std::unordered_map<ArrayConstructorSig, sem::CallTarget*> array_ctors_;
    std::unordered_map<StructConstructorSig, sem::CallTarget*> struct_ctors_;

    sem::Function* current_function_ = nullptr;
    sem::Statement* current_statement_ = nullptr;
    sem::CompoundStatement* current_compound_statement_ = nullptr;
    sem::BlockStatement* current_block_ = nullptr;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_RESOLVER_H_
