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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_TABLE_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_TABLE_H_

#include <memory>
#include <string>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/function.h"
#include "src/tint/lang/core/intrinsic/ctor_conv.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/parameter_usage.h"
#include "src/tint/lang/core/unary_op.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::diag {
class List;
}  // namespace tint::diag
namespace tint::type {
class Manager;
}  // namespace tint::type

namespace tint::core::intrinsic {

/// Table is a lookup table of all the WGSL builtin functions and intrinsic operators
class Table {
  public:
    /// @param data the intrinsic table data
    /// @param types the type manager
    /// @param symbols the symbol table
    /// @param diags the diagnostic list to append errors to
    /// @return a pointer to a newly created Table
    static std::unique_ptr<Table> Create(const TableData& data,
                                         core::type::Manager& types,
                                         SymbolTable& symbols,
                                         diag::List& diags);

    /// Destructor
    virtual ~Table();

    /// Overload describes a fully matched builtin function overload
    struct Overload {
        /// Parameter describes a single parameter
        struct Parameter {
            /// Parameter type
            const core::type::Type* const type;
            /// Parameter usage
            core::ParameterUsage const usage = core::ParameterUsage::kNone;

            /// Equality operator
            /// @param other the parameter to compare against
            /// @returns true if this parameter and @p other are the same
            bool operator==(const Parameter& other) const {
                return type == other.type && usage == other.usage;
            }

            /// Inequality operator
            /// @param other the parameter to compare against
            /// @returns false if this parameter and @p other are the same
            bool operator!=(const Parameter& other) const { return !(*this == other); }
        };

        /// The overload information
        const OverloadInfo* info = nullptr;

        /// The resolved overload return type
        core::type::Type const* return_type = nullptr;

        /// The resolved overload parameters
        Vector<Parameter, 8> parameters;

        /// The constant evaluation function
        constant::Eval::Function const_eval_fn = nullptr;

        /// Equality operator
        /// @param other the overload to compare against
        /// @returns true if this overload and @p other are the same
        bool operator==(const Overload& other) const {
            return info == other.info && return_type == other.return_type &&
                   parameters == other.parameters;
        }

        /// Inequality operator
        /// @param other the overload to compare against
        /// @returns false if this overload and @p other are the same
        bool operator!=(const Overload& other) const { return !(*this == other); }
    };

    /// Lookup looks for the builtin overload with the given signature, raising an error diagnostic
    /// if the builtin was not found.
    /// @param type the builtin type
    /// @param args the argument types passed to the builtin function
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the builtin can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is `EvaluationStage::kRuntime`, then
    ///        only overloads with concrete argument types will be considered, as all
    ///        abstract-numerics will have been materialized after shader creation time
    ///        (EvaluationStage::kConstant).
    /// @param source the source of the builtin call
    /// @return the resolved builtin function overload
    virtual Result<Overload> Lookup(core::Function type,
                                    VectorRef<const core::type::Type*> args,
                                    EvaluationStage earliest_eval_stage,
                                    const Source& source) = 0;

    /// Lookup looks for the unary op overload with the given signature, raising an error
    /// diagnostic if the operator was not found.
    /// @param op the unary operator
    /// @param arg the type of the expression passed to the operator
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the unary operator can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (EvaluationStage::kConstant).
    /// @param source the source of the operator call
    /// @return the resolved unary operator overload
    virtual Result<Overload> Lookup(core::UnaryOp op,
                                    const core::type::Type* arg,
                                    EvaluationStage earliest_eval_stage,
                                    const Source& source) = 0;

    /// Lookup looks for the binary op overload with the given signature, raising an error
    /// diagnostic if the operator was not found.
    /// @param op the binary operator
    /// @param lhs the LHS value type passed to the operator
    /// @param rhs the RHS value type passed to the operator
    /// @param source the source of the operator call
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the binary operator can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (EvaluationStage::kConstant).
    /// @param is_compound true if the binary operator is being used as a compound assignment
    /// @return the resolved binary operator overload
    virtual Result<Overload> Lookup(core::BinaryOp op,
                                    const core::type::Type* lhs,
                                    const core::type::Type* rhs,
                                    EvaluationStage earliest_eval_stage,
                                    const Source& source,
                                    bool is_compound) = 0;

    /// Lookup looks for the value constructor or conversion overload for the given CtorConv.
    /// @param type the type being constructed or converted
    /// @param template_arg the optional template argument
    /// @param args the argument types passed to the constructor / conversion call
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the constructor or conversion can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (EvaluationStage::kConstant).
    /// @param source the source of the call
    /// @return the resolved type constructor or conversion function overload
    virtual Result<Overload> Lookup(CtorConv type,
                                    const core::type::Type* template_arg,
                                    VectorRef<const core::type::Type*> args,
                                    EvaluationStage earliest_eval_stage,
                                    const Source& source) = 0;
};

}  // namespace tint::core::intrinsic

namespace tint {

/// Hasher specialization for core::intrinsic::Table::Overload
template <>
struct Hasher<core::intrinsic::Table::Overload> {
    /// @param i the core::intrinsic::Table::Overload to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const core::intrinsic::Table::Overload& i) const {
        size_t hash = Hash(i.parameters.Length());
        for (auto& p : i.parameters) {
            hash = HashCombine(hash, p.type, p.usage);
        }
        return Hash(hash, i.info, i.return_type);
    }
};

}  // namespace tint

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TABLE_H_
