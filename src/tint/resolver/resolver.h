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

#include "src/tint/constant/value.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/const_eval.h"
#include "src/tint/resolver/dependency_graph.h"
#include "src/tint/resolver/intrinsic_table.h"
#include "src/tint/resolver/sem_helper.h"
#include "src/tint/resolver/validator.h"
#include "src/tint/sem/binding_point.h"
#include "src/tint/sem/block_statement.h"
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
class BlockStatement;
class Builtin;
class CaseStatement;
class ForLoopStatement;
class IfStatement;
class LoopStatement;
class Statement;
class StructMember;
class SwitchStatement;
class WhileStatement;
}  // namespace tint::sem
namespace tint::type {
class Atomic;
}  // namespace tint::type

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

    /// @returns the list of diagnostics raised by the generator.
    const diag::List& Diagnostics() const { return diagnostics_; }

    /// @returns true if the resolver was successful
    bool Resolve();

    /// @param type the given type
    /// @returns true if the given type is a plain type
    bool IsPlain(const type::Type* type) const { return validator_.IsPlain(type); }

    /// @param type the given type
    /// @returns true if the given type is a fixed-footprint type
    bool IsFixedFootprint(const type::Type* type) const {
        return validator_.IsFixedFootprint(type);
    }

    /// @param type the given type
    /// @returns true if the given type is storable
    bool IsStorable(const type::Type* type) const { return validator_.IsStorable(type); }

    /// @param type the given type
    /// @returns true if the given type is host-shareable
    bool IsHostShareable(const type::Type* type) const { return validator_.IsHostShareable(type); }

    /// @returns the validator for testing
    const Validator* GetValidatorForTesting() const { return &validator_; }

  private:
    /// Resolves the program, without creating final the semantic nodes.
    /// @returns true on success, false on error
    bool ResolveInternal();

    /// Creates the nodes and adds them to the sem::Info mappings of the
    /// ProgramBuilder.
    void CreateSemanticNodes() const;

    /// @returns the call of Expression() cast to a sem::ValueExpression. If the sem::Expression is
    /// not a sem::ValueExpression, then an error diagnostic is raised and nullptr is returned.
    sem::ValueExpression* ValueExpression(const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::TypeExpression. If the sem::Expression is
    /// not a sem::TypeExpression, then an error diagnostic is raised and nullptr is returned.
    sem::TypeExpression* TypeExpression(const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::FunctionExpression. If the sem::Expression
    /// is not a sem::FunctionExpression, then an error diagnostic is raised and nullptr is
    /// returned.
    sem::FunctionExpression* FunctionExpression(const ast::Expression* expr);

    /// @returns the resolved type from an expression, or nullptr on error
    type::Type* Type(const ast::Expression* ast);

    /// @returns the call of Expression() cast to a
    /// sem::BuiltinEnumExpression<builtin::AddressSpace>. If the sem::Expression is not a
    /// sem::BuiltinEnumExpression<builtin::AddressSpace>, then an error diagnostic is raised and
    /// nullptr is returned.
    sem::BuiltinEnumExpression<builtin::AddressSpace>* AddressSpaceExpression(
        const ast::Expression* expr);

    /// @returns the call of Expression() cast to a
    /// sem::BuiltinEnumExpression<builtin::BuiltinValue>. If the sem::Expression is not a
    /// sem::BuiltinEnumExpression<builtin::BuiltinValue>, then an error diagnostic is raised and
    /// nullptr is returned.
    sem::BuiltinEnumExpression<builtin::BuiltinValue>* BuiltinValueExpression(
        const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::BuiltinEnumExpression<type::TexelFormat>.
    /// If the sem::Expression is not a sem::BuiltinEnumExpression<type::TexelFormat>, then an error
    /// diagnostic is raised and nullptr is returned.
    sem::BuiltinEnumExpression<builtin::TexelFormat>* TexelFormatExpression(
        const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::BuiltinEnumExpression<builtin::Access>*.
    /// If the sem::Expression is not a sem::BuiltinEnumExpression<builtin::Access>*, then an error
    /// diagnostic is raised and nullptr is returned.
    sem::BuiltinEnumExpression<builtin::Access>* AccessExpression(const ast::Expression* expr);

    /// @returns the call of Expression() cast to a
    /// sem::BuiltinEnumExpression<builtin::InterpolationSampling>*. If the sem::Expression is not a
    /// sem::BuiltinEnumExpression<builtin::InterpolationSampling>*, then an error diagnostic is
    /// raised and nullptr is returned.
    sem::BuiltinEnumExpression<builtin::InterpolationSampling>* InterpolationSampling(
        const ast::Expression* expr);

    /// @returns the call of Expression() cast to a
    /// sem::BuiltinEnumExpression<builtin::InterpolationType>*. If the sem::Expression is not a
    /// sem::BuiltinEnumExpression<builtin::InterpolationType>*, then an error diagnostic is raised
    /// and nullptr is returned.
    sem::BuiltinEnumExpression<builtin::InterpolationType>* InterpolationType(
        const ast::Expression* expr);

    /// Expression traverses the graph of expressions starting at `expr`, building a post-ordered
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
    sem::ValueExpression* IndexAccessor(const ast::IndexAccessorExpression*);
    sem::ValueExpression* Binary(const ast::BinaryExpression*);
    sem::ValueExpression* Bitcast(const ast::BitcastExpression*);
    sem::Call* Call(const ast::CallExpression*);
    sem::Function* Function(const ast::Function*);
    template <size_t N>
    sem::Call* FunctionCall(const ast::CallExpression*,
                            sem::Function* target,
                            utils::Vector<const sem::ValueExpression*, N>& args,
                            sem::Behaviors arg_behaviors);
    sem::Expression* Identifier(const ast::IdentifierExpression*);
    template <size_t N>
    sem::Call* BuiltinCall(const ast::CallExpression*,
                           builtin::Function,
                           utils::Vector<const sem::ValueExpression*, N>& args);
    sem::ValueExpression* Literal(const ast::LiteralExpression*);
    sem::ValueExpression* MemberAccessor(const ast::MemberAccessorExpression*);
    sem::ValueExpression* UnaryOp(const ast::UnaryOpExpression*);

    /// Register a memory store to an expression, to track accesses to root identifiers in order to
    /// perform alias analysis.
    void RegisterStore(const sem::ValueExpression* expr);

    /// Perform pointer alias analysis for `call`.
    /// @returns true is the call arguments are free from aliasing issues, false otherwise.
    bool AliasAnalysis(const sem::Call* call);

    /// If `expr` is of a reference type, then Load will create and return a sem::Load node wrapping
    /// `expr`. If `expr` is not of a reference type, then Load will just return `expr`.
    const sem::ValueExpression* Load(const sem::ValueExpression* expr);

    /// If `expr` is not of an abstract-numeric type, then Materialize() will just return `expr`.
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
    /// If `expr` is not of an abstract-numeric type, then Materialize() will just return `expr`.
    /// If `expr` is nullptr, then Materialize() will also return nullptr.
    const sem::ValueExpression* Materialize(const sem::ValueExpression* expr,
                                            const type::Type* target_type = nullptr);

    /// For each argument in `args`:
    /// * Calls Materialize() passing the argument and the corresponding parameter type.
    /// * Calls Load() passing the argument, iff the corresponding parameter type is not a
    ///   reference type.
    /// @returns true on success, false on failure.
    template <size_t N>
    bool MaybeMaterializeAndLoadArguments(utils::Vector<const sem::ValueExpression*, N>& args,
                                          const sem::CallTarget* target);

    /// @returns true if an argument of an abstract numeric type, passed to a parameter of type
    /// `parameter_ty` should be materialized.
    bool ShouldMaterializeArgument(const type::Type* parameter_ty) const;

    /// Converts `c` to `target_ty`
    /// @returns true on success, false on failure.
    bool Convert(const constant::Value*& c, const type::Type* target_ty, const Source& source);

    /// Transforms `args` to a vector of constants, and converts each constant to the call target's
    /// parameter type.
    /// @returns the vector of constants, `utils::Failure` on failure.
    template <size_t N>
    utils::Result<utils::Vector<const constant::Value*, N>> ConvertArguments(
        const utils::Vector<const sem::ValueExpression*, N>& args,
        const sem::CallTarget* target);

    /// @param ty the type that may hold abstract numeric types
    /// @param target_ty the target type for the expression (variable type, parameter type, etc).
    ///        May be nullptr.
    /// @param source the source of the expression requiring materialization
    /// @returns the concrete (materialized) type for the given type, or nullptr if the type is
    ///          already concrete.
    const type::Type* ConcreteType(const type::Type* ty,
                                   const type::Type* target_ty,
                                   const Source& source);

    // Statement resolving methods
    // Each return true on success, false on failure.
    sem::Statement* AssignmentStatement(const ast::AssignmentStatement*);
    sem::BlockStatement* BlockStatement(const ast::BlockStatement*);
    sem::Statement* BreakStatement(const ast::BreakStatement*);
    sem::Statement* BreakIfStatement(const ast::BreakIfStatement*);
    sem::Statement* CallStatement(const ast::CallStatement*);
    sem::CaseStatement* CaseStatement(const ast::CaseStatement*, const type::Type*);
    sem::Statement* CompoundAssignmentStatement(const ast::CompoundAssignmentStatement*);
    sem::Statement* ContinueStatement(const ast::ContinueStatement*);
    sem::Statement* ConstAssert(const ast::ConstAssert*);
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
    sem::SwitchStatement* SwitchStatement(const ast::SwitchStatement* s);
    sem::Statement* VariableDeclStatement(const ast::VariableDeclStatement*);
    bool Statements(utils::VectorRef<const ast::Statement*>);

    // CollectTextureSamplerPairs() collects all the texture/sampler pairs from the target function
    // / builtin, and records these on the current function by calling AddTextureSamplerPair().
    void CollectTextureSamplerPairs(sem::Function* func,
                                    utils::VectorRef<const sem::ValueExpression*> args) const;
    void CollectTextureSamplerPairs(const sem::Builtin* builtin,
                                    utils::VectorRef<const sem::ValueExpression*> args) const;

    /// Resolves the WorkgroupSize for the given function, assigning it to
    /// current_function_
    bool WorkgroupSize(const ast::Function*);

    /// Resolves the `@builtin` attribute @p attr
    /// @returns the builtin value on success
    utils::Result<tint::builtin::BuiltinValue> BuiltinAttribute(const ast::BuiltinAttribute* attr);

    /// Resolves the `@location` attribute @p attr
    /// @returns the location value on success.
    utils::Result<uint32_t> LocationAttribute(const ast::LocationAttribute* attr);

    /// Resolves the `@binding` attribute @p attr
    /// @returns the binding value on success.
    utils::Result<uint32_t> BindingAttribute(const ast::BindingAttribute* attr);

    /// Resolves the `@group` attribute @p attr
    /// @returns the group value on success.
    utils::Result<uint32_t> GroupAttribute(const ast::GroupAttribute* attr);

    /// Resolves the `@workgroup_size` attribute @p attr
    /// @returns the workgroup size on success.
    utils::Result<sem::WorkgroupSize> WorkgroupAttribute(const ast::WorkgroupAttribute* attr);

    /// Resolves the `@diagnostic` attribute @p attr
    /// @returns true on success, false on failure
    bool DiagnosticAttribute(const ast::DiagnosticAttribute* attr);

    /// Resolves the stage attribute @p attr
    /// @returns true on success, false on failure
    bool StageAttribute(const ast::StageAttribute* attr);

    /// Resolves the `@must_use` attribute @p attr
    /// @returns true on success, false on failure
    bool MustUseAttribute(const ast::MustUseAttribute* attr);

    /// Resolves the `@invariant` attribute @p attr
    /// @returns true on success, false on failure
    bool InvariantAttribute(const ast::InvariantAttribute*);

    /// Resolves the `@stride` attribute @p attr
    /// @returns true on success, false on failure
    bool StrideAttribute(const ast::StrideAttribute*);

    /// Resolves the `@interpolate` attribute @p attr
    /// @returns true on success, false on failure
    utils::Result<builtin::Interpolation> InterpolateAttribute(
        const ast::InterpolateAttribute* attr);

    /// Resolves the internal attribute @p attr
    /// @returns true on success, false on failure
    bool InternalAttribute(const ast::InternalAttribute* attr);

    /// @param control the diagnostic control
    /// @returns true on success, false on failure
    bool DiagnosticControl(const ast::DiagnosticControl& control);

    /// @param enable the enable declaration
    /// @returns the resolved extension
    bool Enable(const ast::Enable* enable);

    /// @param named_type the named type to resolve
    /// @returns the resolved semantic type
    type::Type* TypeDecl(const ast::TypeDecl* named_type);

    /// Resolves and validates the expression used as the count parameter of an array.
    /// @param count_expr the expression used as the second template parameter to an array<>.
    /// @returns the number of elements in the array.
    const type::ArrayCount* ArrayCount(const ast::Expression* count_expr);

    /// Resolves and validates the attributes on an array.
    /// @param attributes the attributes on the array type.
    /// @param el_ty the element type of the array.
    /// @param explicit_stride assigned the specified stride of the array in bytes.
    /// @returns true on success, false on failure
    bool ArrayAttributes(utils::VectorRef<const ast::Attribute*> attributes,
                         const type::Type* el_ty,
                         uint32_t& explicit_stride);

    /// Builds and returns the semantic information for an array.
    /// @returns the semantic Array information, or nullptr if an error is raised.
    /// @param array_source the source of the array
    /// @param el_source the source of the array element, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @param count_source the source of the array count, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @param el_ty the Array element type
    /// @param el_count the number of elements in the array.
    /// @param explicit_stride the explicit byte stride of the array. Zero means implicit stride.
    type::Array* Array(const Source& array_source,
                       const Source& el_source,
                       const Source& count_source,
                       const type::Type* el_ty,
                       const type::ArrayCount* el_count,
                       uint32_t explicit_stride);

    /// Builds and returns the semantic information for the alias `alias`.
    /// This method does not mark the ast::Alias node, nor attach the generated
    /// semantic information to the AST node.
    /// @returns the aliased type, or nullptr if an error is raised.
    type::Type* Alias(const ast::Alias* alias);

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
    /// @param func the AST function that owns the parameter
    /// @param index the index of the parameter
    sem::Parameter* Parameter(const ast::Parameter* param,
                              const ast::Function* func,
                              uint32_t index);

    /// Records the address space usage for the given type, and any transient
    /// dependencies of the type. Validates that the type can be used for the
    /// given address space, erroring if it cannot.
    /// @param sc the address space to apply to the type and transitent types
    /// @param ty the type to apply the address space on
    /// @param usage the Source of the root variable declaration that uses the
    /// given type and address space. Used for generating sensible error
    /// messages.
    /// @returns true on success, false on error
    bool ApplyAddressSpaceUsageToType(builtin::AddressSpace sc,
                                      type::Type* ty,
                                      const Source& usage);

    /// @param address_space the address space
    /// @returns the default access control for the given address space
    builtin::Access DefaultAccessForAddressSpace(builtin::AddressSpace address_space);

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

    /// Applies the diagnostic severities from the current scope to a semantic node.
    /// @param node the semantic node to apply the diagnostic severities to
    template <typename NODE>
    void ApplyDiagnosticSeverities(NODE* node);

    /// Checks @p ident is not an ast::TemplatedIdentifier.
    /// If @p ident is a ast::TemplatedIdentifier, then an error diagnostic is raised.
    /// @returns true if @p ident is not a ast::TemplatedIdentifier.
    bool CheckNotTemplated(const char* use, const ast::Identifier* ident);

    /// Raises an error diagnostic that the resolved identifier @p resolved was not of the expected
    /// kind.
    /// @param source the source of the error diagnostic
    /// @param resolved the resolved identifier
    /// @param wanted the expected kind
    void ErrorMismatchedResolvedIdentifier(const Source& source,
                                           const ResolvedIdentifier& resolved,
                                           std::string_view wanted);

    /// Raises an error that the attribute is not valid for the given use.
    /// @param attr the invalue attribute
    /// @param use the thing that the attribute was applied to
    void ErrorInvalidAttribute(const ast::Attribute* attr, std::string_view use);

    /// Adds the given error message to the diagnostics
    void AddError(const std::string& msg, const Source& source) const;

    /// Adds the given warning message to the diagnostics
    void AddWarning(const std::string& msg, const Source& source) const;

    /// Adds the given note message to the diagnostics
    void AddNote(const std::string& msg, const Source& source) const;

    /// @returns the type::Type for the builtin type @p builtin_ty with the identifier @p ident
    /// @note: Will raise an ICE if @p symbol is not a builtin type.
    type::Type* BuiltinType(builtin::Builtin builtin_ty, const ast::Identifier* ident);

    /// @returns the nesting depth of @ty as defined in
    /// https://gpuweb.github.io/gpuweb/wgsl/#composite-types
    size_t NestDepth(const type::Type* ty) const;

    // ArrayConstructorSig represents a unique array constructor signature.
    // It is a tuple of the array type, number of arguments provided and earliest evaluation stage.
    using ArrayConstructorSig =
        utils::UnorderedKeyWrapper<std::tuple<const type::Array*, size_t, sem::EvaluationStage>>;

    // StructConstructorSig represents a unique structure constructor signature.
    // It is a tuple of the structure type, number of arguments provided and earliest evaluation
    // stage.
    using StructConstructorSig =
        utils::UnorderedKeyWrapper<std::tuple<const type::Struct*, size_t, sem::EvaluationStage>>;

    /// ExprEvalStageConstraint describes a constraint on when expressions can be evaluated.
    struct ExprEvalStageConstraint {
        /// The latest stage that the expression can be evaluated
        sem::EvaluationStage stage = sem::EvaluationStage::kRuntime;
        /// The 'thing' that is imposing the contraint. e.g. "var declaration"
        /// If nullptr, then there is no constraint
        const char* constraint = nullptr;
    };

    /// AliasAnalysisInfo captures the memory accesses performed by a given function for the purpose
    /// of determining if any two arguments alias at any callsite.
    struct AliasAnalysisInfo {
        /// The set of module-scope variables that are written to, and where that write occurs.
        std::unordered_map<const sem::Variable*, const sem::ValueExpression*> module_scope_writes;
        /// The set of module-scope variables that are read from, and where that read occurs.
        std::unordered_map<const sem::Variable*, const sem::ValueExpression*> module_scope_reads;
        /// The set of function parameters that are written to.
        std::unordered_set<const sem::Variable*> parameter_writes;
        /// The set of function parameters that are read from.
        std::unordered_set<const sem::Variable*> parameter_reads;
    };

    /// A hint for the usage of an identifier expression.
    /// Used to provide more informative error diagnostics on resolution failure.
    struct IdentifierResolveHint {
        /// The expression this hint applies to
        const ast::Expression* expression = nullptr;
        /// The usage of the identifier.
        const char* usage = "identifier";
        /// Suggested strings if the identifier failed to resolve
        utils::Slice<char const* const> suggestions = utils::Empty;
    };

    ProgramBuilder* const builder_;
    diag::List& diagnostics_;
    ConstEval const_eval_;
    std::unique_ptr<IntrinsicTable> const intrinsic_table_;
    DependencyGraph dependencies_;
    SemHelper sem_;
    Validator validator_;
    builtin::Extensions enabled_extensions_;
    utils::Vector<sem::Function*, 8> entry_points_;
    utils::Hashmap<const type::Type*, const Source*, 8> atomic_composite_info_;
    utils::Bitset<0> marked_;
    ExprEvalStageConstraint expr_eval_stage_constraint_;
    std::unordered_map<const sem::Function*, AliasAnalysisInfo> alias_analysis_infos_;
    utils::Hashmap<OverrideId, const sem::Variable*, 8> override_ids_;
    utils::Hashmap<ArrayConstructorSig, sem::CallTarget*, 8> array_ctors_;
    utils::Hashmap<StructConstructorSig, sem::CallTarget*, 8> struct_ctors_;
    sem::Function* current_function_ = nullptr;
    sem::Statement* current_statement_ = nullptr;
    sem::CompoundStatement* current_compound_statement_ = nullptr;
    uint32_t current_scoping_depth_ = 0;
    utils::UniqueVector<const sem::GlobalVariable*, 4>* resolved_overrides_ = nullptr;
    utils::Hashset<TypeAndAddressSpace, 8> valid_type_storage_layouts_;
    utils::Hashmap<const ast::Expression*, const ast::BinaryExpression*, 8>
        logical_binary_lhs_to_parent_;
    utils::Hashset<const ast::Expression*, 8> skip_const_eval_;
    IdentifierResolveHint identifier_resolve_hint_;
    utils::Hashmap<const type::Type*, size_t, 8> nest_depth_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_RESOLVER_H_
