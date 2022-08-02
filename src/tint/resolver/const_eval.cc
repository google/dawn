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

#include "src/tint/resolver/const_eval.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/bool.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/f16.h"
#include "src/tint/sem/f32.h"
#include "src/tint/sem/i32.h"
#include "src/tint/sem/matrix.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/u32.h"
#include "src/tint/sem/vector.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/transform.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {

namespace {

/// Returns the first element of a parameter pack
template <typename T>
T First(T&& first, ...) {
    return std::forward<T>(first);
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Assumes all `cs` are of the same type.
template <typename F, typename... CONSTANTS>
auto Dispatch_ia_iu32(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const sem::AbstractInt*) { return f(cs->template As<AInt>()...); },
        [&](const sem::I32*) { return f(cs->template As<i32>()...); },
        [&](const sem::U32*) { return f(cs->template As<u32>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Assumes all `cs` are of the same type.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fi32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const sem::AbstractInt*) { return f(cs->template As<AInt>()...); },
        [&](const sem::AbstractFloat*) { return f(cs->template As<AFloat>()...); },
        [&](const sem::F32*) { return f(cs->template As<f32>()...); },
        [&](const sem::I32*) { return f(cs->template As<i32>()...); },
        [&](const sem::F16*) { return f(cs->template As<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Assumes all `cs` are of the same type.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fiu32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const sem::AbstractInt*) { return f(cs->template As<AInt>()...); },
        [&](const sem::AbstractFloat*) { return f(cs->template As<AFloat>()...); },
        [&](const sem::F32*) { return f(cs->template As<f32>()...); },
        [&](const sem::I32*) { return f(cs->template As<i32>()...); },
        [&](const sem::U32*) { return f(cs->template As<u32>()...); },
        [&](const sem::F16*) { return f(cs->template As<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Assumes all `cs` are of the same type.
template <typename F, typename... CONSTANTS>
auto Dispatch_fa_f32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const sem::AbstractFloat*) { return f(cs->template As<AFloat>()...); },
        [&](const sem::F32*) { return f(cs->template As<f32>()...); },
        [&](const sem::F16*) { return f(cs->template As<f16>()...); });
}

/// ZeroTypeDispatch is a helper for calling the function `f`, passing a single zero-value argument
/// of the C++ type that corresponds to the sem::Type `type`. For example, calling
/// `ZeroTypeDispatch()` with a type of `sem::I32*` will call the function f with a single argument
/// of `i32(0)`.
/// @returns the value returned by calling `f`.
/// @note `type` must be a scalar or abstract numeric type. Other types will not call `f`, and will
/// return the zero-initialized value of the return type for `f`.
template <typename F>
auto ZeroTypeDispatch(const sem::Type* type, F&& f) {
    return Switch(
        type,                                                     //
        [&](const sem::AbstractInt*) { return f(AInt(0)); },      //
        [&](const sem::AbstractFloat*) { return f(AFloat(0)); },  //
        [&](const sem::I32*) { return f(i32(0)); },               //
        [&](const sem::U32*) { return f(u32(0)); },               //
        [&](const sem::F32*) { return f(f32(0)); },               //
        [&](const sem::F16*) { return f(f16(0)); },               //
        [&](const sem::Bool*) { return f(static_cast<bool>(0)); });
}

/// @returns `value` if `T` is not a Number, otherwise ValueOf returns the inner value of the
/// Number.
template <typename T>
inline auto ValueOf(T value) {
    if constexpr (std::is_same_v<UnwrapNumber<T>, T>) {
        return value;
    } else {
        return value.value;
    }
}

/// @returns true if `value` is a positive zero.
template <typename T>
inline bool IsPositiveZero(T value) {
    using N = UnwrapNumber<T>;
    return Number<N>(value) == Number<N>(0);  // Considers sign bit
}

/// Constant inherits from sem::Constant to add an private implementation method for conversion.
struct Constant : public sem::Constant {
    /// Convert attempts to convert the constant value to the given type. On error, Convert()
    /// creates a new diagnostic message and returns a Failure.
    virtual utils::Result<const Constant*> Convert(ProgramBuilder& builder,
                                                   const sem::Type* target_ty,
                                                   const Source& source) const = 0;
};

// Forward declaration
const Constant* CreateComposite(ProgramBuilder& builder,
                                const sem::Type* type,
                                utils::VectorRef<const sem::Constant*> elements);

/// Element holds a single scalar or abstract-numeric value.
/// Element implements the Constant interface.
template <typename T>
struct Element : Constant {
    static_assert(!std::is_same_v<UnwrapNumber<T>, T> || std::is_same_v<T, bool>,
                  "T must be a Number or bool");

    Element(const sem::Type* t, T v) : type(t), value(v) {}
    ~Element() override = default;
    const sem::Type* Type() const override { return type; }
    std::variant<std::monostate, AInt, AFloat> Value() const override {
        if constexpr (IsFloatingPoint<UnwrapNumber<T>>) {
            return static_cast<AFloat>(value);
        } else {
            return static_cast<AInt>(value);
        }
    }
    const sem::Constant* Index(size_t) const override { return nullptr; }
    bool AllZero() const override { return IsPositiveZero(value); }
    bool AnyZero() const override { return IsPositiveZero(value); }
    bool AllEqual() const override { return true; }
    size_t Hash() const override { return utils::Hash(type, ValueOf(value)); }

    utils::Result<const Constant*> Convert(ProgramBuilder& builder,
                                           const sem::Type* target_ty,
                                           const Source& source) const override {
        TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);
        if (target_ty == type) {
            // If the types are identical, then no conversion is needed.
            return this;
        }
        bool failed = false;
        auto* res = ZeroTypeDispatch(target_ty, [&](auto zero_to) -> const Constant* {
            // `T` is the source type, `value` is the source value.
            // `TO` is the target type.
            using TO = std::decay_t<decltype(zero_to)>;
            if constexpr (std::is_same_v<TO, bool>) {
                // [x -> bool]
                return builder.create<Element<TO>>(target_ty, !IsPositiveZero(value));
            } else if constexpr (std::is_same_v<T, bool>) {
                // [bool -> x]
                return builder.create<Element<TO>>(target_ty, TO(value ? 1 : 0));
            } else if (auto conv = CheckedConvert<TO>(value)) {
                // Conversion success
                return builder.create<Element<TO>>(target_ty, conv.Get());
                // --- Below this point are the failure cases ---
            } else if constexpr (std::is_same_v<T, AInt> || std::is_same_v<T, AFloat>) {
                // [abstract-numeric -> x] - materialization failure
                std::stringstream ss;
                ss << "value " << value << " cannot be represented as ";
                ss << "'" << builder.FriendlyName(target_ty) << "'";
                builder.Diagnostics().add_error(tint::diag::System::Resolver, ss.str(), source);
                failed = true;
            } else if constexpr (IsFloatingPoint<UnwrapNumber<TO>>) {
                // [x -> floating-point] - number not exactly representable
                // https://www.w3.org/TR/WGSL/#floating-point-conversion
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<Element<TO>>(target_ty, -TO::Inf());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<Element<TO>>(target_ty, TO::Inf());
                }
            } else {
                // [x -> integer] - number not exactly representable
                // https://www.w3.org/TR/WGSL/#floating-point-conversion
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<Element<TO>>(target_ty, TO::Lowest());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<Element<TO>>(target_ty, TO::Highest());
                }
            }
            return nullptr;  // Expression is not constant.
        });
        if (failed) {
            // A diagnostic error has been raised, and resolving should abort.
            return utils::Failure;
        }
        return res;
        TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
    }

    sem::Type const* const type;
    const T value;
};

/// Splat holds a single Constant value, duplicated as all children.
/// Splat is used for zero-initializers, 'splat' constructors, or constructors where each element is
/// identical. Splat may be of a vector, matrix or array type.
/// Splat implements the Constant interface.
struct Splat : Constant {
    Splat(const sem::Type* t, const sem::Constant* e, size_t n) : type(t), el(e), count(n) {}
    ~Splat() override = default;
    const sem::Type* Type() const override { return type; }
    std::variant<std::monostate, AInt, AFloat> Value() const override { return {}; }
    const sem::Constant* Index(size_t i) const override { return i < count ? el : nullptr; }
    bool AllZero() const override { return el->AllZero(); }
    bool AnyZero() const override { return el->AnyZero(); }
    bool AllEqual() const override { return true; }
    size_t Hash() const override { return utils::Hash(type, el->Hash(), count); }

    utils::Result<const Constant*> Convert(ProgramBuilder& builder,
                                           const sem::Type* target_ty,
                                           const Source& source) const override {
        // Convert the single splatted element type.
        // Note: This file is the only place where `sem::Constant`s are created, so this static_cast
        // is safe.
        auto conv_el = static_cast<const Constant*>(el)->Convert(
            builder, sem::Type::ElementOf(target_ty), source);
        if (!conv_el) {
            return utils::Failure;
        }
        if (!conv_el.Get()) {
            return nullptr;
        }
        return builder.create<Splat>(target_ty, conv_el.Get(), count);
    }

    sem::Type const* const type;
    const sem::Constant* el;
    const size_t count;
};

/// Composite holds a number of mixed child Constant values.
/// Composite may be of a vector, matrix or array type.
/// If each element is the same type and value, then a Splat would be a more efficient constant
/// implementation. Use CreateComposite() to create the appropriate Constant type.
/// Composite implements the Constant interface.
struct Composite : Constant {
    Composite(const sem::Type* t,
              utils::VectorRef<const sem::Constant*> els,
              bool all_0,
              bool any_0)
        : type(t), elements(std::move(els)), all_zero(all_0), any_zero(any_0), hash(CalcHash()) {}
    ~Composite() override = default;
    const sem::Type* Type() const override { return type; }
    std::variant<std::monostate, AInt, AFloat> Value() const override { return {}; }
    const sem::Constant* Index(size_t i) const override {
        return i < elements.Length() ? elements[i] : nullptr;
    }
    bool AllZero() const override { return all_zero; }
    bool AnyZero() const override { return any_zero; }
    bool AllEqual() const override { return false; /* otherwise this should be a Splat */ }
    size_t Hash() const override { return hash; }

    utils::Result<const Constant*> Convert(ProgramBuilder& builder,
                                           const sem::Type* target_ty,
                                           const Source& source) const override {
        // Convert each of the composite element types.
        auto* el_ty = sem::Type::ElementOf(target_ty);
        utils::Vector<const sem::Constant*, 4> conv_els;
        conv_els.Reserve(elements.Length());
        for (auto* el : elements) {
            // Note: This file is the only place where `sem::Constant`s are created, so this
            // static_cast is safe.
            auto conv_el = static_cast<const Constant*>(el)->Convert(builder, el_ty, source);
            if (!conv_el) {
                return utils::Failure;
            }
            if (!conv_el.Get()) {
                return nullptr;
            }
            conv_els.Push(conv_el.Get());
        }
        return CreateComposite(builder, target_ty, std::move(conv_els));
    }

    size_t CalcHash() {
        auto h = utils::Hash(type, all_zero, any_zero);
        for (auto* el : elements) {
            utils::HashCombine(&h, el->Hash());
        }
        return h;
    }

    sem::Type const* const type;
    const utils::Vector<const sem::Constant*, 8> elements;
    const bool all_zero;
    const bool any_zero;
    const size_t hash;
};

/// CreateElement constructs and returns an Element<T>.
template <typename T>
const Constant* CreateElement(ProgramBuilder& builder, const sem::Type* t, T v) {
    return builder.create<Element<T>>(t, v);
}

/// ZeroValue returns a Constant for the zero-value of the type `type`.
const Constant* ZeroValue(ProgramBuilder& builder, const sem::Type* type) {
    return Switch(
        type,  //
        [&](const sem::Vector* v) -> const Constant* {
            auto* zero_el = ZeroValue(builder, v->type());
            return builder.create<Splat>(type, zero_el, v->Width());
        },
        [&](const sem::Matrix* m) -> const Constant* {
            auto* zero_el = ZeroValue(builder, m->ColumnType());
            return builder.create<Splat>(type, zero_el, m->columns());
        },
        [&](const sem::Array* a) -> const Constant* {
            if (auto* zero_el = ZeroValue(builder, a->ElemType())) {
                return builder.create<Splat>(type, zero_el, a->Count());
            }
            return nullptr;
        },
        [&](const sem::Struct* s) -> const Constant* {
            std::unordered_map<const sem::Type*, const Constant*> zero_by_type;
            utils::Vector<const sem::Constant*, 4> zeros;
            zeros.Reserve(s->Members().size());
            for (auto* member : s->Members()) {
                auto* zero = utils::GetOrCreate(zero_by_type, member->Type(),
                                                [&] { return ZeroValue(builder, member->Type()); });
                if (!zero) {
                    return nullptr;
                }
                zeros.Push(zero);
            }
            if (zero_by_type.size() == 1) {
                // All members were of the same type, so the zero value is the same for all members.
                return builder.create<Splat>(type, zeros[0], s->Members().size());
            }
            return CreateComposite(builder, s, std::move(zeros));
        },
        [&](Default) -> const Constant* {
            return ZeroTypeDispatch(type, [&](auto zero) -> const Constant* {
                return CreateElement(builder, type, zero);
            });
        });
}

/// Equal returns true if the constants `a` and `b` are of the same type and value.
bool Equal(const sem::Constant* a, const sem::Constant* b) {
    if (a->Hash() != b->Hash()) {
        return false;
    }
    if (a->Type() != b->Type()) {
        return false;
    }
    return Switch(
        a->Type(),  //
        [&](const sem::Vector* vec) {
            for (size_t i = 0; i < vec->Width(); i++) {
                if (!Equal(a->Index(i), b->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](const sem::Matrix* mat) {
            for (size_t i = 0; i < mat->columns(); i++) {
                if (!Equal(a->Index(i), b->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](const sem::Array* arr) {
            for (size_t i = 0; i < arr->Count(); i++) {
                if (!Equal(a->Index(i), b->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](Default) { return a->Value() == b->Value(); });
}

/// CreateComposite is used to construct a constant of a vector, matrix or array type.
/// CreateComposite examines the element values and will return either a Composite or a Splat,
/// depending on the element types and values.
const Constant* CreateComposite(ProgramBuilder& builder,
                                const sem::Type* type,
                                utils::VectorRef<const sem::Constant*> elements) {
    if (elements.IsEmpty()) {
        return nullptr;
    }
    bool any_zero = false;
    bool all_zero = true;
    bool all_equal = true;
    auto* first = elements.Front();
    for (auto* el : elements) {
        if (!el) {
            return nullptr;
        }
        if (!any_zero && el->AnyZero()) {
            any_zero = true;
        }
        if (all_zero && !el->AllZero()) {
            all_zero = false;
        }
        if (all_equal && el != first) {
            if (!Equal(el, first)) {
                all_equal = false;
            }
        }
    }
    if (all_equal) {
        return builder.create<Splat>(type, elements[0], elements.Length());
    } else {
        return builder.create<Composite>(type, std::move(elements), all_zero, any_zero);
    }
}

/// TransformElements constructs a new constant by applying the transformation function 'f' on each
/// of the most deeply nested elements of 'cs'.
template <typename F, typename... CONSTANTS>
const Constant* TransformElements(ProgramBuilder& builder, F&& f, CONSTANTS&&... cs) {
    uint32_t n = 0;
    auto* ty = First(cs...)->Type();
    auto* el_ty = sem::Type::ElementOf(ty, &n);
    if (el_ty == ty) {
        return f(cs...);
    }
    utils::Vector<const sem::Constant*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        els.Push(TransformElements(builder, f, cs->Index(i)...));
    }
    return CreateComposite(builder, ty, std::move(els));
}

}  // namespace

ConstEval::ConstEval(ProgramBuilder& b) : builder(b) {}

const sem::Constant* ConstEval::Literal(const sem::Type* ty,
                                        const ast::LiteralExpression* literal) {
    return Switch(
        literal,
        [&](const ast::BoolLiteralExpression* lit) {
            return CreateElement(builder, ty, lit->value);
        },
        [&](const ast::IntLiteralExpression* lit) -> const Constant* {
            switch (lit->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                    return CreateElement(builder, ty, AInt(lit->value));
                case ast::IntLiteralExpression::Suffix::kI:
                    return CreateElement(builder, ty, i32(lit->value));
                case ast::IntLiteralExpression::Suffix::kU:
                    return CreateElement(builder, ty, u32(lit->value));
            }
            return nullptr;
        },
        [&](const ast::FloatLiteralExpression* lit) -> const Constant* {
            switch (lit->suffix) {
                case ast::FloatLiteralExpression::Suffix::kNone:
                    return CreateElement(builder, ty, AFloat(lit->value));
                case ast::FloatLiteralExpression::Suffix::kF:
                    return CreateElement(builder, ty, f32(lit->value));
                case ast::FloatLiteralExpression::Suffix::kH:
                    return CreateElement(builder, ty, f16(lit->value));
            }
            return nullptr;
        });
}

const sem::Constant* ConstEval::ArrayOrStructCtor(const sem::Type* ty,
                                                  utils::VectorRef<const sem::Expression*> args) {
    if (args.IsEmpty()) {
        return ZeroValue(builder, ty);
    }

    if (args.Length() == 1 && args[0]->Type() == ty) {
        // Identity constructor.
        return args[0]->ConstantValue();
    }

    // Multiple arguments. Must be a type constructor.
    utils::Vector<const sem::Constant*, 4> els;
    els.Reserve(args.Length());
    for (auto* arg : args) {
        els.Push(arg->ConstantValue());
    }
    return CreateComposite(builder, ty, std::move(els));
}

const sem::Constant* ConstEval::Conv(const sem::Type* ty,
                                     utils::VectorRef<const sem::Expression*> args) {
    uint32_t el_count = 0;
    auto* el_ty = sem::Type::ElementOf(ty, &el_count);
    if (!el_ty) {
        return nullptr;
    }

    auto& src = args[0]->Declaration()->source;
    auto* arg = args[0]->ConstantValue();
    if (!arg) {
        return nullptr;  // Single argument is not constant.
    }

    if (auto conv = Convert(ty, arg, src)) {
        return conv.Get();
    }

    return nullptr;
}

const sem::Constant* ConstEval::Zero(const sem::Type* ty,
                                     utils::VectorRef<const sem::Expression*>) {
    return ZeroValue(builder, ty);
}

const sem::Constant* ConstEval::Identity(const sem::Type*,
                                         utils::VectorRef<const sem::Expression*> args) {
    return args[0]->ConstantValue();
}

const sem::Constant* ConstEval::VecSplat(const sem::Type* ty,
                                         utils::VectorRef<const sem::Expression*> args) {
    if (auto* arg = args[0]->ConstantValue()) {
        return builder.create<Splat>(ty, arg, static_cast<const sem::Vector*>(ty)->Width());
    }
    return nullptr;
}

const sem::Constant* ConstEval::VecCtorS(const sem::Type* ty,
                                         utils::VectorRef<const sem::Expression*> args) {
    utils::Vector<const sem::Constant*, 4> els;
    for (auto* arg : args) {
        els.Push(arg->ConstantValue());
    }
    return CreateComposite(builder, ty, std::move(els));
}

const sem::Constant* ConstEval::VecCtorM(const sem::Type* ty,
                                         utils::VectorRef<const sem::Expression*> args) {
    utils::Vector<const sem::Constant*, 4> els;
    for (auto* arg : args) {
        auto* val = arg->ConstantValue();
        if (!val) {
            return nullptr;
        }
        auto* arg_ty = arg->Type();
        if (auto* arg_vec = arg_ty->As<sem::Vector>()) {
            // Extract out vector elements.
            for (uint32_t j = 0; j < arg_vec->Width(); j++) {
                auto* el = val->Index(j);
                if (!el) {
                    return nullptr;
                }
                els.Push(el);
            }
        } else {
            els.Push(val);
        }
    }
    return CreateComposite(builder, ty, std::move(els));
}

const sem::Constant* ConstEval::MatCtorS(const sem::Type* ty,
                                         utils::VectorRef<const sem::Expression*> args) {
    auto* m = static_cast<const sem::Matrix*>(ty);

    utils::Vector<const sem::Constant*, 4> els;
    for (uint32_t c = 0; c < m->columns(); c++) {
        utils::Vector<const sem::Constant*, 4> column;
        for (uint32_t r = 0; r < m->rows(); r++) {
            auto i = r + c * m->rows();
            column.Push(args[i]->ConstantValue());
        }
        els.Push(CreateComposite(builder, m->ColumnType(), std::move(column)));
    }
    return CreateComposite(builder, ty, std::move(els));
}

const sem::Constant* ConstEval::MatCtorV(const sem::Type* ty,
                                         utils::VectorRef<const sem::Expression*> args) {
    utils::Vector<const sem::Constant*, 4> els;
    for (auto* arg : args) {
        els.Push(arg->ConstantValue());
    }
    return CreateComposite(builder, ty, std::move(els));
}

const sem::Constant* ConstEval::Index(const sem::Expression* obj_expr,
                                      const sem::Expression* idx_expr) {
    auto obj_val = obj_expr->ConstantValue();
    if (!obj_val) {
        return {};
    }

    auto idx_val = idx_expr->ConstantValue();
    if (!idx_val) {
        return {};
    }

    uint32_t el_count = 0;
    sem::Type::ElementOf(obj_val->Type(), &el_count);

    AInt idx = idx_val->As<AInt>();
    if (idx < 0 || idx >= el_count) {
        auto clamped = std::min<AInt::type>(std::max<AInt::type>(idx, 0), el_count - 1);
        AddWarning("index " + std::to_string(idx) + " out of bounds [0.." +
                       std::to_string(el_count - 1) + "]. Clamping index to " +
                       std::to_string(clamped),
                   idx_expr->Declaration()->source);
        idx = clamped;
    }

    return obj_val->Index(static_cast<size_t>(idx));
}

const sem::Constant* ConstEval::MemberAccess(const sem::Expression* obj_expr,
                                             const sem::StructMember* member) {
    auto obj_val = obj_expr->ConstantValue();
    if (!obj_val) {
        return {};
    }
    return obj_val->Index(static_cast<size_t>(member->Index()));
}

const sem::Constant* ConstEval::Swizzle(const sem::Type* ty,
                                        const sem::Expression* vec_expr,
                                        utils::VectorRef<uint32_t> indices) {
    auto* vec_val = vec_expr->ConstantValue();
    if (!vec_val) {
        return nullptr;
    }
    if (indices.Length() == 1) {
        return vec_val->Index(static_cast<size_t>(indices[0]));
    } else {
        auto values = utils::Transform<4>(
            indices, [&](uint32_t i) { return vec_val->Index(static_cast<size_t>(i)); });
        return CreateComposite(builder, ty, std::move(values));
    }
}

const sem::Constant* ConstEval::Bitcast(const sem::Type*, const sem::Expression*) {
    // TODO(crbug.com/tint/1581): Implement @const intrinsics
    return nullptr;
}

const sem::Constant* ConstEval::OpComplement(const sem::Type*,
                                             utils::VectorRef<const sem::Expression*> args) {
    auto transform = [&](const sem::Constant* c) {
        auto create = [&](auto i) {
            return CreateElement(builder, c->Type(), decltype(i)(~i.value));
        };
        return Dispatch_ia_iu32(create, c);
    };
    return TransformElements(builder, transform, args[0]->ConstantValue());
}

const sem::Constant* ConstEval::OpMinus(const sem::Type*,
                                        utils::VectorRef<const sem::Expression*> args) {
    auto transform = [&](const sem::Constant* c) {
        auto create = [&](auto i) {  //
                                     // For signed integrals, avoid C++ UB by not negating the
                                     // smallest negative number. In WGSL, this operation is well
                                     // defined to return the same value, see:
                                     // https://gpuweb.github.io/gpuweb/wgsl/#arithmetic-expr.
            using T = UnwrapNumber<decltype(i)>;
            if constexpr (std::is_integral_v<T>) {
                auto v = i.value;
                if (v != std::numeric_limits<T>::min()) {
                    v = -v;
                }
                return CreateElement(builder, c->Type(), decltype(i)(v));
            } else {
                return CreateElement(builder, c->Type(), decltype(i)(-i.value));
            }
        };
        return Dispatch_fia_fi32_f16(create, c);
    };
    return TransformElements(builder, transform, args[0]->ConstantValue());
}

const sem::Constant* ConstEval::atan2(const sem::Type*,
                                      utils::VectorRef<const sem::Expression*> args) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) {
            return CreateElement(builder, c0->Type(), decltype(i)(std::atan2(i.value, j.value)));
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformElements(builder, transform, args[0]->ConstantValue(),
                             args[1]->ConstantValue());
}

const sem::Constant* ConstEval::clamp(const sem::Type*,
                                      utils::VectorRef<const sem::Expression*> args) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1,
                         const sem::Constant* c2) {
        auto create = [&](auto e, auto low, auto high) {
            return CreateElement(builder, c0->Type(),
                                 decltype(e)(std::min(std::max(e, low), high)));
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1, c2);
    };
    return TransformElements(builder, transform, args[0]->ConstantValue(), args[1]->ConstantValue(),
                             args[2]->ConstantValue());
}

utils::Result<const sem::Constant*> ConstEval::Convert(const sem::Type* target_ty,
                                                       const sem::Constant* value,
                                                       const Source& source) {
    if (value->Type() == target_ty) {
        return value;
    }
    auto conv = static_cast<const Constant*>(value)->Convert(builder, target_ty, source);
    if (!conv) {
        return utils::Failure;
    }
    return conv.Get();
}

void ConstEval::AddError(const std::string& msg, const Source& source) const {
    builder.Diagnostics().add_error(diag::System::Resolver, msg, source);
}

void ConstEval::AddWarning(const std::string& msg, const Source& source) const {
    builder.Diagnostics().add_warning(diag::System::Resolver, msg, source);
}

}  // namespace tint::resolver
