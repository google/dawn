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
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/resolver/const_eval.h"
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
#include "src/tint/utils/bitset.h"
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
class WhileStatement;
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
class StructMember;
class SwitchStatement;
class TypeInitializer;
class WhileStatement;
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
    Validator::ValidTypeStorageLayouts valid_type_storage_layouts_;

    /// Resolves the program, without creating final the semantic nodes.
    /// @returns true on success, false on error
    bool ResolveInternal();

    /// Creates the nodes and adds them to the sem::Info mappings of the
    /// ProgramBuilder.
    void CreateSemanticNodes() const;

    /// Expression traverses the graph of expressions starting at `expr`, building a postordered
    /// list (leaf-first) of all the expression nodes. Each of the expressions are then resolved by
    /// dispatching to the appropriate expression handlers below.
    /// @returns the resolved semantic node for the expression `expr`, or nullptr on failure.
    sem::Expression* Expression(const ast::Expression* expr);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Expression resolving methods
    //
    // Returns the semantic node pointer on success, nullptr on failure.
    //
    // These methods are invoked by Expression(), in postorder (child-first). These methods should
    // not attempt to resolve their children. This design avoids recursion, which is a common cause
    // of stack-overflows.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    sem::Expression* IndexAccessor(const ast::IndexAccessorExpression*);
    sem::Expression* Binary(const ast::BinaryExpression*);
    sem::Expression* Bitcast(const ast::BitcastExpression*);
    sem::Call* Call(const ast::CallExpression*);
    sem::Function* Function(const ast::Function*);
    template <size_t N>
    sem::Call* FunctionCall(const ast::CallExpression*,
                            sem::Function* target,
                            utils::Vector<const sem::Expression*, N>& args,
                            sem::Behaviors arg_behaviors);
    sem::Expression* Identifier(const ast::IdentifierExpression*);
    template <size_t N>
    sem::Call* BuiltinCall(const ast::CallExpression*,
                           sem::BuiltinType,
                           utils::Vector<const sem::Expression*, N>& args);
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
    template <size_t N>
    bool MaybeMaterializeArguments(utils::Vector<const sem::Expression*, N>& args,
                                   const sem::CallTarget* target);

    /// @returns true if an argument of an abstract numeric type, passed to a parameter of type
    /// `parameter_ty` should be materialized.
    bool ShouldMaterializeArgument(const sem::Type* parameter_ty) const;

    /// Converts `c` to `target_ty`
    /// @returns true on success, false on failure.
    bool Convert(const sem::Constant*& c, const sem::Type* target_ty, const Source& source);

    /// Transforms `args` to a vector of constants, and converts each constant to the call target's
    /// parameter type.
    /// @returns the vector of constants, `utils::Failure` on failure.
    template <size_t N>
    utils::Result<utils::Vector<const sem::Constant*, N>> ConvertArguments(
        const utils::Vector<const sem::Expression*, N>& args,
        const sem::CallTarget* target);

    /// @param ty the type that may hold abstract numeric types
    /// @param target_ty the target type for the expression (variable type, parameter type, etc).
    ///        May be nullptr.
    /// @param source the source of the expression requiring materialization
    /// @returns the concrete (materialized) type for the given type, or nullptr if the type is
    ///          already concrete.
    const sem::Type* ConcreteType(const sem::Type* ty,
                                  const sem::Type* target_ty,
                                  const Source& source);

    // Statement resolving methods
    // Each return true on success, false on failure.
    sem::Statement* AssignmentStatement(const ast::AssignmentStatement*);
    sem::BlockStatement* BlockStatement(const ast::BlockStatement*);
    sem::Statement* BreakStatement(const ast::BreakStatement*);
    sem::Statement* BreakIfStatement(const ast::BreakIfStatement*);
    sem::Statement* CallStatement(const ast::CallStatement*);
    sem::CaseStatement* CaseStatement(const ast::CaseStatement*, const sem::Type*);
    sem::Statement* CompoundAssignmentStatement(const ast::CompoundAssignmentStatement*);
    sem::Statement* ContinueStatement(const ast::ContinueStatement*);
    sem::Statement* DiscardStatement(const ast::DiscardStatement*);
    sem::ForLoopStatement* ForLoopStatement(const ast::ForLoopStatement*);
    sem::WhileStatement* WhileStatement(const ast::WhileStatement*);
    sem::GlobalVariable* GlobalVariable(const ast::Variable*);
    sem::Statement* Parameter(const ast::Variable*);
    sem::IfStatement* IfStatement(const ast::IfStatement*);
    sem::Statement* IncrementDecrementStatement(const ast::IncrementDecrementStatement*);
    sem::LoopStatement* LoopStatement(const ast::LoopStatement*);
    sem::Statement* ReturnStatement(const ast::ReturnStatement*);
    sem::Statement* Statement(const ast::Statement*);
    sem::Statement* StaticAssert(const ast::StaticAssert*);
    sem::SwitchStatement* SwitchStatement(const ast::SwitchStatement* s);
    sem::Statement* VariableDeclStatement(const ast::VariableDeclStatement*);
    bool Statements(utils::VectorRef<const ast::Statement*>);

    // CollectTextureSamplerPairs() collects all the texture/sampler pairs from the target function
    // / builtin, and records these on the current function by calling AddTextureSamplerPair().
    void CollectTextureSamplerPairs(sem::Function* func,
                                    utils::VectorRef<const sem::Expression*> args) const;
    void CollectTextureSamplerPairs(const sem::Builtin* builtin,
                                    utils::VectorRef<const sem::Expression*> args) const;

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

    /// Builds and returns the semantic information for the AST array `arr`.
    /// This method does not mark the ast::Array node, nor attach the generated semantic information
    /// to the AST node.
    /// @returns the semantic Array information, or nullptr if an error is raised.
    /// @param arr the Array to get semantic information for
    sem::Array* Array(const ast::Array* arr);

    /// Resolves and validates the expression used as the count parameter of an array.
    /// @param count_expr the expression used as the second template parameter to an array<>.
    /// @returns the number of elements in the array.
    utils::Result<sem::ArrayCount> ArrayCount(const ast::Expression* count_expr);

    /// Resolves and validates the attributes on an array.
    /// @param attributes the attributes on the array type.
    /// @param el_ty the element type of the array.
    /// @param explicit_stride assigned the specified stride of the array in bytes.
    /// @returns true on success, false on failure
    bool ArrayAttributes(utils::VectorRef<const ast::Attribute*> attributes,
                         const sem::Type* el_ty,
                         uint32_t& explicit_stride);

    /// Builds and returns the semantic information for an array.
    /// @returns the semantic Array information, or nullptr if an error is raised.
    /// @param el_source the source of the array element, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @param count_source the source of the array count, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @param el_ty the Array element type
    /// @param el_count the number of elements in the array.
    /// @param explicit_stride the explicit byte stride of the array. Zero means implicit stride.
    sem::Array* Array(const Source& el_source,
                      const Source& count_source,
                      const sem::Type* el_ty,
                      sem::ArrayCount el_count,
                      uint32_t explicit_stride);

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

    /// @returns the semantic info for the variable `v`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param var the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Variable(const ast::Variable* var, bool is_global);

    /// @returns the semantic info for the `ast::Let` `v`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param var the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Let(const ast::Let* var, bool is_global);

    /// @returns the semantic info for the module-scope `ast::Override` `v`. If an error is raised,
    /// nullptr is returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param override the variable
    sem::Variable* Override(const ast::Override* override);

    /// @returns the semantic info for an `ast::Const` `v`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param const_ the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Const(const ast::Const* const_, bool is_global);

    /// @returns the semantic info for the `ast::Var` `var`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param var the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Var(const ast::Var* var, bool is_global);

    /// @returns the semantic info for the function parameter `param`. If an error is raised,
    /// nullptr is returned.
    /// @note the caller is expected to validate the parameter
    /// @param param the AST parameter
    /// @param index the index of the parameter
    sem::Parameter* Parameter(const ast::Parameter* param, uint32_t index);

    /// @returns the location value for a `@location` attribute, validating the value's range and
    /// type.
    utils::Result<uint32_t> LocationAttribute(const ast::LocationAttribute* attr);

    /// Records the address space usage for the given type, and any transient
    /// dependencies of the type. Validates that the type can be used for the
    /// given address space, erroring if it cannot.
    /// @param sc the address space to apply to the type and transitent types
    /// @param ty the type to apply the address space on
    /// @param usage the Source of the root variable declaration that uses the
    /// given type and address space. Used for generating sensible error
    /// messages.
    /// @returns true on success, false on error
    bool ApplyAddressSpaceUsageToType(ast::AddressSpace sc, sem::Type* ty, const Source& usage);

    /// @param address_space the address space
    /// @returns the default access control for the given address space
    ast::Access DefaultAccessForAddressSpace(ast::AddressSpace address_space);

    /// Allocate constant IDs for pipeline-overridable constants.
    /// @returns true on success, false on error
    bool AllocateOverridableConstantIds();

    /// Set the shadowing information on variable declarations.
    /// @note this method must only be called after all semantic nodes are built.
    void SetShadows();
    /// StatementScope() does the following:
    /// * Creates the AST -> SEM mapping.
    /// * Assigns `sem` to #current_statement_
    /// * Assigns `sem` to #current_compound_statement_ if `sem` derives from
    ///   sem::CompoundStatement.
    /// * Then calls `callback`.
    /// * Before returning #current_statement_ and #current_compound_statement_ are restored to
    /// their original values.
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

    /// @returns true if the symbol is the name of a builtin function.
    bool IsBuiltin(Symbol) const;

    // ArrayInitializerSig represents a unique array initializer signature.
    // It is a tuple of the array type, number of arguments provided and earliest evaluation stage.
    using ArrayInitializerSig =
        utils::UnorderedKeyWrapper<std::tuple<const sem::Array*, size_t, sem::EvaluationStage>>;

    // StructInitializerSig represents a unique structure initializer signature.
    // It is a tuple of the structure type, number of arguments provided and earliest evaluation
    // stage.
    using StructInitializerSig =
        utils::UnorderedKeyWrapper<std::tuple<const sem::Struct*, size_t, sem::EvaluationStage>>;

    /// ExprEvalStageConstraint describes a constraint on when expressions can be evaluated.
    struct ExprEvalStageConstraint {
        /// The latest stage that the expression can be evaluated
        sem::EvaluationStage stage = sem::EvaluationStage::kRuntime;
        /// The 'thing' that is imposing the contraint. e.g. "var declaration"
        /// If nullptr, then there is no constraint
        const char* constraint = nullptr;
    };

    ProgramBuilder* const builder_;
    diag::List& diagnostics_;
    ConstEval const_eval_;
    std::unique_ptr<IntrinsicTable> const intrinsic_table_;
    DependencyGraph dependencies_;
    SemHelper sem_;
    Validator validator_;
    ast::Extensions enabled_extensions_;
    utils::Vector<sem::Function*, 8> entry_points_;
    utils::Hashmap<const sem::Type*, const Source*, 8> atomic_composite_info_;
    utils::Bitset<0> marked_;
    ExprEvalStageConstraint expr_eval_stage_constraint_;
    utils::Hashmap<OverrideId, const sem::Variable*, 8> override_ids_;
    utils::Hashmap<ArrayInitializerSig, sem::CallTarget*, 8> array_inits_;
    utils::Hashmap<StructInitializerSig, sem::CallTarget*, 8> struct_inits_;
    sem::Function* current_function_ = nullptr;
    sem::Statement* current_statement_ = nullptr;
    sem::CompoundStatement* current_compound_statement_ = nullptr;
    uint32_t current_scoping_depth_ = 0;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_RESOLVER_H_
