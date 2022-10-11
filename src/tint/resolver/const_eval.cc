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
#include "src/tint/utils/scoped_assignment.h"
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
auto Dispatch_ia_iu32_bool(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const sem::AbstractInt*) { return f(cs->template As<AInt>()...); },
        [&](const sem::I32*) { return f(cs->template As<i32>()...); },
        [&](const sem::U32*) { return f(cs->template As<u32>()...); },
        [&](const sem::Bool*) { return f(cs->template As<bool>()...); });
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
auto Dispatch_fia_fiu32_f16_bool(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const sem::AbstractInt*) { return f(cs->template As<AInt>()...); },
        [&](const sem::AbstractFloat*) { return f(cs->template As<AFloat>()...); },
        [&](const sem::F32*) { return f(cs->template As<f32>()...); },
        [&](const sem::I32*) { return f(cs->template As<i32>()...); },
        [&](const sem::U32*) { return f(cs->template As<u32>()...); },
        [&](const sem::F16*) { return f(cs->template As<f16>()...); },
        [&](const sem::Bool*) { return f(cs->template As<bool>()...); });
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

/// Helper that calls `f` passing in the value of all `cs`.
/// Assumes all `cs` are of the same type.
template <typename F, typename... CONSTANTS>
auto Dispatch_bool(F&& f, CONSTANTS&&... cs) {
    return f(cs->template As<bool>()...);
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

template <typename NumberT>
std::string OverflowErrorMessage(NumberT lhs, const char* op, NumberT rhs) {
    std::stringstream ss;
    ss << "'" << lhs.value << " " << op << " " << rhs.value << "' cannot be represented as '"
       << FriendlyName<NumberT>() << "'";
    return ss.str();
}

/// ImplConstant inherits from sem::Constant to add an private implementation method for conversion.
struct ImplConstant : public sem::Constant {
    /// Convert attempts to convert the constant value to the given type. On error, Convert()
    /// creates a new diagnostic message and returns a Failure.
    virtual utils::Result<const ImplConstant*> Convert(ProgramBuilder& builder,
                                                       const sem::Type* target_ty,
                                                       const Source& source) const = 0;
};

/// A result templated with a ImplConstant.
using ImplResult = utils::Result<const ImplConstant*>;

// Forward declaration
const ImplConstant* CreateComposite(ProgramBuilder& builder,
                                    const sem::Type* type,
                                    utils::VectorRef<const sem::Constant*> elements);

/// Element holds a single scalar or abstract-numeric value.
/// Element implements the Constant interface.
template <typename T>
struct Element : ImplConstant {
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

    ImplResult Convert(ProgramBuilder& builder,
                       const sem::Type* target_ty,
                       const Source& source) const override {
        TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);
        if (target_ty == type) {
            // If the types are identical, then no conversion is needed.
            return this;
        }
        return ZeroTypeDispatch(target_ty, [&](auto zero_to) -> ImplResult {
            // `value` is the source value.
            // `FROM` is the source type.
            // `TO` is the target type.
            using TO = std::decay_t<decltype(zero_to)>;
            using FROM = T;
            if constexpr (std::is_same_v<TO, bool>) {
                // [x -> bool]
                return builder.create<Element<TO>>(target_ty, !IsPositiveZero(value));
            } else if constexpr (std::is_same_v<FROM, bool>) {
                // [bool -> x]
                return builder.create<Element<TO>>(target_ty, TO(value ? 1 : 0));
            } else if (auto conv = CheckedConvert<TO>(value)) {
                // Conversion success
                return builder.create<Element<TO>>(target_ty, conv.Get());
                // --- Below this point are the failure cases ---
            } else if constexpr (IsAbstract<FROM>) {
                // [abstract-numeric -> x] - materialization failure
                std::stringstream ss;
                ss << "value " << value << " cannot be represented as ";
                ss << "'" << builder.FriendlyName(target_ty) << "'";
                builder.Diagnostics().add_error(tint::diag::System::Resolver, ss.str(), source);
                return utils::Failure;
            } else if constexpr (IsFloatingPoint<TO>) {
                // [x -> floating-point] - number not exactly representable
                // https://www.w3.org/TR/WGSL/#floating-point-conversion
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<Element<TO>>(target_ty, -TO::Inf());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<Element<TO>>(target_ty, TO::Inf());
                }
            } else if constexpr (IsFloatingPoint<FROM>) {
                // [floating-point -> integer] - number not exactly representable
                // https://www.w3.org/TR/WGSL/#floating-point-conversion
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<Element<TO>>(target_ty, TO::Lowest());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<Element<TO>>(target_ty, TO::Highest());
                }
            } else if constexpr (IsIntegral<FROM>) {
                // [integer -> integer] - number not exactly representable
                // Static cast
                return builder.create<Element<TO>>(target_ty, static_cast<TO>(value));
            }
            return nullptr;  // Expression is not constant.
        });
        TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
    }

    sem::Type const* const type;
    const T value;
};

/// Splat holds a single Constant value, duplicated as all children.
/// Splat is used for zero-initializers, 'splat' constructors, or constructors where each element is
/// identical. Splat may be of a vector, matrix or array type.
/// Splat implements the Constant interface.
struct Splat : ImplConstant {
    Splat(const sem::Type* t, const sem::Constant* e, size_t n) : type(t), el(e), count(n) {}
    ~Splat() override = default;
    const sem::Type* Type() const override { return type; }
    std::variant<std::monostate, AInt, AFloat> Value() const override { return {}; }
    const sem::Constant* Index(size_t i) const override { return i < count ? el : nullptr; }
    bool AllZero() const override { return el->AllZero(); }
    bool AnyZero() const override { return el->AnyZero(); }
    bool AllEqual() const override { return true; }
    size_t Hash() const override { return utils::Hash(type, el->Hash(), count); }

    ImplResult Convert(ProgramBuilder& builder,
                       const sem::Type* target_ty,
                       const Source& source) const override {
        // Convert the single splatted element type.
        // Note: This file is the only place where `sem::Constant`s are created, so this static_cast
        // is safe.
        auto conv_el = static_cast<const ImplConstant*>(el)->Convert(
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
struct Composite : ImplConstant {
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

    ImplResult Convert(ProgramBuilder& builder,
                       const sem::Type* target_ty,
                       const Source& source) const override {
        // Convert each of the composite element types.
        auto* el_ty = sem::Type::ElementOf(target_ty);
        utils::Vector<const sem::Constant*, 4> conv_els;
        conv_els.Reserve(elements.Length());
        for (auto* el : elements) {
            // Note: This file is the only place where `sem::Constant`s are created, so this
            // static_cast is safe.
            auto conv_el = static_cast<const ImplConstant*>(el)->Convert(builder, el_ty, source);
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
            h = utils::HashCombine(h, el->Hash());
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
const ImplConstant* CreateElement(ProgramBuilder& builder, const sem::Type* t, T v) {
    return builder.create<Element<T>>(t, v);
}

/// ZeroValue returns a Constant for the zero-value of the type `type`.
const ImplConstant* ZeroValue(ProgramBuilder& builder, const sem::Type* type) {
    return Switch(
        type,  //
        [&](const sem::Vector* v) -> const ImplConstant* {
            auto* zero_el = ZeroValue(builder, v->type());
            return builder.create<Splat>(type, zero_el, v->Width());
        },
        [&](const sem::Matrix* m) -> const ImplConstant* {
            auto* zero_el = ZeroValue(builder, m->ColumnType());
            return builder.create<Splat>(type, zero_el, m->columns());
        },
        [&](const sem::Array* a) -> const ImplConstant* {
            if (auto n = a->ConstantCount()) {
                if (auto* zero_el = ZeroValue(builder, a->ElemType())) {
                    return builder.create<Splat>(type, zero_el, n.value());
                }
            }
            return nullptr;
        },
        [&](const sem::Struct* s) -> const ImplConstant* {
            std::unordered_map<const sem::Type*, const ImplConstant*> zero_by_type;
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
        [&](Default) -> const ImplConstant* {
            return ZeroTypeDispatch(type, [&](auto zero) -> const ImplConstant* {
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
            if (auto count = arr->ConstantCount()) {
                for (size_t i = 0; i < count; i++) {
                    if (!Equal(a->Index(i), b->Index(i))) {
                        return false;
                    }
                }
                return true;
            }

            return false;
        },
        [&](Default) { return a->Value() == b->Value(); });
}

/// CreateComposite is used to construct a constant of a vector, matrix or array type.
/// CreateComposite examines the element values and will return either a Composite or a Splat,
/// depending on the element types and values.
const ImplConstant* CreateComposite(ProgramBuilder& builder,
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

namespace detail {
/// Implementation of TransformElements
template <typename F, typename... CONSTANTS>
ImplResult TransformElements(ProgramBuilder& builder,
                             const sem::Type* composite_ty,
                             F&& f,
                             size_t index,
                             CONSTANTS&&... cs) {
    uint32_t n = 0;
    auto* ty = First(cs...)->Type();
    auto* el_ty = sem::Type::ElementOf(ty, &n);
    if (el_ty == ty) {
        constexpr bool kHasIndexParam = traits::IsType<size_t, traits::LastParameterType<F>>;
        if constexpr (kHasIndexParam) {
            return f(cs..., index);
        } else {
            return f(cs...);
        }
    }
    utils::Vector<const sem::Constant*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        if (auto el = detail::TransformElements(builder, sem::Type::ElementOf(composite_ty),
                                                std::forward<F>(f), index + i, cs->Index(i)...)) {
            els.Push(el.Get());

        } else {
            return el.Failure();
        }
    }
    return CreateComposite(builder, composite_ty, std::move(els));
}
}  // namespace detail

/// TransformElements constructs a new constant of type `composite_ty` by applying the
/// transformation function `f` on each of the most deeply nested elements of 'cs'. Assumes that all
/// input constants `cs` are of the same arity (all scalars or all vectors of the same size).
/// If `f`'s last argument is a `size_t`, then the index of the most deeply nested element inside
/// the most deeply nested aggregate type will be passed in.
template <typename F, typename... CONSTANTS>
ImplResult TransformElements(ProgramBuilder& builder,
                             const sem::Type* composite_ty,
                             F&& f,
                             CONSTANTS&&... cs) {
    return detail::TransformElements(builder, composite_ty, f, 0, cs...);
}

/// TransformBinaryElements constructs a new constant of type `composite_ty` by applying the
/// transformation function 'f' on each of the most deeply nested elements of both `c0` and `c1`.
/// Unlike TransformElements, this function handles the constants being of different arity, e.g.
/// vector-scalar, scalar-vector.
template <typename F>
ImplResult TransformBinaryElements(ProgramBuilder& builder,
                                   const sem::Type* composite_ty,
                                   F&& f,
                                   const sem::Constant* c0,
                                   const sem::Constant* c1) {
    uint32_t n0 = 0, n1 = 0;
    sem::Type::ElementOf(c0->Type(), &n0);
    sem::Type::ElementOf(c1->Type(), &n1);
    uint32_t max_n = std::max(n0, n1);
    // If arity of both constants is 1, invoke callback
    if (max_n == 1u) {
        return f(c0, c1);
    }

    utils::Vector<const sem::Constant*, 8> els;
    els.Reserve(max_n);
    for (uint32_t i = 0; i < max_n; i++) {
        auto nested_or_self = [&](auto& c, uint32_t num_elems) {
            if (num_elems == 1) {
                return c;
            }
            return c->Index(i);
        };
        if (auto el = TransformBinaryElements(builder, sem::Type::ElementOf(composite_ty),
                                              std::forward<F>(f), nested_or_self(c0, n0),
                                              nested_or_self(c1, n1))) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return CreateComposite(builder, composite_ty, std::move(els));
}
}  // namespace

ConstEval::ConstEval(ProgramBuilder& b) : builder(b) {}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Add(NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT>) {
        // Check for over/underflow for abstract values
        if (auto r = CheckedAdd(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "+", b), *current_source);
            return utils::Failure;
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto add_values = [](T lhs, T rhs) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // Ensure no UB for signed overflow
                using UT = std::make_unsigned_t<T>;
                return static_cast<T>(static_cast<UT>(lhs) + static_cast<UT>(rhs));
            } else {
                return lhs + rhs;
            }
        };
        result = add_values(a.value, b.value);
    }
    return result;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Mul(NumberT a, NumberT b) {
    using T = UnwrapNumber<NumberT>;
    NumberT result;
    if constexpr (IsAbstract<NumberT>) {
        // Check for over/underflow for abstract values
        if (auto r = CheckedMul(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "*", b), *current_source);
            return utils::Failure;
        }
    } else {
        auto mul_values = [](T lhs, T rhs) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // For signed integrals, avoid C++ UB by multiplying as unsigned
                using UT = std::make_unsigned_t<T>;
                return static_cast<T>(static_cast<UT>(lhs) * static_cast<UT>(rhs));
            } else {
                return lhs * rhs;
            }
        };
        result = mul_values(a.value, b.value);
    }
    return result;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Dot2(NumberT a1, NumberT a2, NumberT b1, NumberT b2) {
    auto r1 = Mul(a1, b1);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(a2, b2);
    if (!r2) {
        return utils::Failure;
    }
    auto r = Add(r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Dot3(NumberT a1,
                                       NumberT a2,
                                       NumberT a3,
                                       NumberT b1,
                                       NumberT b2,
                                       NumberT b3) {
    auto r1 = Mul(a1, b1);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(a2, b2);
    if (!r2) {
        return utils::Failure;
    }
    auto r3 = Mul(a3, b3);
    if (!r3) {
        return utils::Failure;
    }
    auto r = Add(r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(r.Get(), r3.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Dot4(NumberT a1,
                                       NumberT a2,
                                       NumberT a3,
                                       NumberT a4,
                                       NumberT b1,
                                       NumberT b2,
                                       NumberT b3,
                                       NumberT b4) {
    auto r1 = Mul(a1, b1);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(a2, b2);
    if (!r2) {
        return utils::Failure;
    }
    auto r3 = Mul(a3, b3);
    if (!r3) {
        return utils::Failure;
    }
    auto r4 = Mul(a4, b4);
    if (!r4) {
        return utils::Failure;
    }
    auto r = Add(r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(r.Get(), r3.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(r.Get(), r4.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

auto ConstEval::AddFunc(const sem::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ImplResult {
        if (auto r = Add(a1, a2)) {
            return CreateElement(builder, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::MulFunc(const sem::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ImplResult {
        if (auto r = Mul(a1, a2)) {
            return CreateElement(builder, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Dot2Func(const sem::Type* elem_ty) {
    return [=](auto a1, auto a2, auto b1, auto b2) -> ImplResult {
        if (auto r = Dot2(a1, a2, b1, b2)) {
            return CreateElement(builder, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Dot3Func(const sem::Type* elem_ty) {
    return [=](auto a1, auto a2, auto a3, auto b1, auto b2, auto b3) -> ImplResult {
        if (auto r = Dot3(a1, a2, a3, b1, b2, b3)) {
            return CreateElement(builder, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Dot4Func(const sem::Type* elem_ty) {
    return
        [=](auto a1, auto a2, auto a3, auto a4, auto b1, auto b2, auto b3, auto b4) -> ImplResult {
            if (auto r = Dot4(a1, a2, a3, a4, b1, b2, b3, b4)) {
                return CreateElement(builder, elem_ty, r.Get());
            }
            return utils::Failure;
        };
}

ConstEval::Result ConstEval::Literal(const sem::Type* ty, const ast::LiteralExpression* literal) {
    return Switch(
        literal,
        [&](const ast::BoolLiteralExpression* lit) {
            return CreateElement(builder, ty, lit->value);
        },
        [&](const ast::IntLiteralExpression* lit) -> ImplResult {
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
        [&](const ast::FloatLiteralExpression* lit) -> ImplResult {
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

ConstEval::Result ConstEval::ArrayOrStructCtor(const sem::Type* ty,
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

ConstEval::Result ConstEval::Conv(const sem::Type* ty,
                                  utils::VectorRef<const sem::Constant*> args,
                                  const Source& source) {
    uint32_t el_count = 0;
    auto* el_ty = sem::Type::ElementOf(ty, &el_count);
    if (!el_ty) {
        return nullptr;
    }

    if (!args[0]) {
        return nullptr;  // Single argument is not constant.
    }

    return Convert(ty, args[0], source);
}

ConstEval::Result ConstEval::Zero(const sem::Type* ty,
                                  utils::VectorRef<const sem::Constant*>,
                                  const Source&) {
    return ZeroValue(builder, ty);
}

ConstEval::Result ConstEval::Identity(const sem::Type*,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source&) {
    return args[0];
}

ConstEval::Result ConstEval::VecSplat(const sem::Type* ty,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source&) {
    if (auto* arg = args[0]) {
        return builder.create<Splat>(ty, arg, static_cast<const sem::Vector*>(ty)->Width());
    }
    return nullptr;
}

ConstEval::Result ConstEval::VecCtorS(const sem::Type* ty,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source&) {
    return CreateComposite(builder, ty, args);
}

ConstEval::Result ConstEval::VecCtorM(const sem::Type* ty,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source&) {
    utils::Vector<const sem::Constant*, 4> els;
    for (auto* arg : args) {
        auto* val = arg;
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

ConstEval::Result ConstEval::MatCtorS(const sem::Type* ty,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source&) {
    auto* m = static_cast<const sem::Matrix*>(ty);

    utils::Vector<const sem::Constant*, 4> els;
    for (uint32_t c = 0; c < m->columns(); c++) {
        utils::Vector<const sem::Constant*, 4> column;
        for (uint32_t r = 0; r < m->rows(); r++) {
            auto i = r + c * m->rows();
            column.Push(args[i]);
        }
        els.Push(CreateComposite(builder, m->ColumnType(), std::move(column)));
    }
    return CreateComposite(builder, ty, std::move(els));
}

ConstEval::Result ConstEval::MatCtorV(const sem::Type* ty,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source&) {
    return CreateComposite(builder, ty, args);
}

ConstEval::Result ConstEval::Index(const sem::Expression* obj_expr,
                                   const sem::Expression* idx_expr) {
    auto idx_val = idx_expr->ConstantValue();
    if (!idx_val) {
        return nullptr;
    }

    uint32_t el_count = 0;
    sem::Type::ElementOf(obj_expr->Type()->UnwrapRef(), &el_count);

    AInt idx = idx_val->As<AInt>();
    if (idx < 0 || (el_count > 0 && idx >= el_count)) {
        std::string range;
        if (el_count > 0) {
            range = " [0.." + std::to_string(el_count - 1) + "]";
        }
        AddError("index " + std::to_string(idx) + " out of bounds" + range,
                 idx_expr->Declaration()->source);
        return utils::Failure;
    }

    auto obj_val = obj_expr->ConstantValue();
    if (!obj_val) {
        return nullptr;
    }

    return obj_val->Index(static_cast<size_t>(idx));
}

ConstEval::Result ConstEval::MemberAccess(const sem::Expression* obj_expr,
                                          const sem::StructMember* member) {
    auto obj_val = obj_expr->ConstantValue();
    if (!obj_val) {
        return nullptr;
    }
    return obj_val->Index(static_cast<size_t>(member->Index()));
}

ConstEval::Result ConstEval::Swizzle(const sem::Type* ty,
                                     const sem::Expression* vec_expr,
                                     utils::VectorRef<uint32_t> indices) {
    auto* vec_val = vec_expr->ConstantValue();
    if (!vec_val) {
        return nullptr;
    }
    if (indices.Length() == 1) {
        return vec_val->Index(static_cast<size_t>(indices[0]));
    }
    auto values = utils::Transform<4>(
        indices, [&](uint32_t i) { return vec_val->Index(static_cast<size_t>(i)); });
    return CreateComposite(builder, ty, std::move(values));
}

ConstEval::Result ConstEval::Bitcast(const sem::Type*, const sem::Expression*) {
    // TODO(crbug.com/tint/1581): Implement @const intrinsics
    return nullptr;
}

ConstEval::Result ConstEval::OpComplement(const sem::Type* ty,
                                          utils::VectorRef<const sem::Constant*> args,
                                          const Source&) {
    auto transform = [&](const sem::Constant* c) {
        auto create = [&](auto i) {
            return CreateElement(builder, c->Type(), decltype(i)(~i.value));
        };
        return Dispatch_ia_iu32(create, c);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::OpUnaryMinus(const sem::Type* ty,
                                          utils::VectorRef<const sem::Constant*> args,
                                          const Source&) {
    auto transform = [&](const sem::Constant* c) {
        auto create = [&](auto i) {
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
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::OpNot(const sem::Type* ty,
                                   utils::VectorRef<const sem::Constant*> args,
                                   const Source&) {
    auto transform = [&](const sem::Constant* c) {
        auto create = [&](auto i) { return CreateElement(builder, c->Type(), decltype(i)(!i)); };
        return Dispatch_bool(create, c);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::OpPlus(const sem::Type* ty,
                                    utils::VectorRef<const sem::Constant*> args,
                                    const Source& source) {
    TINT_SCOPED_ASSIGNMENT(current_source, &source);
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        return Dispatch_fia_fiu32_f16(AddFunc(c0->Type()), c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpMinus(const sem::Type* ty,
                                     utils::VectorRef<const sem::Constant*> args,
                                     const Source& source) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            using NumberT = decltype(i);
            NumberT result;
            if constexpr (IsAbstract<NumberT>) {
                // Check for over/underflow for abstract values
                if (auto r = CheckedSub(i, j)) {
                    result = r->value;
                } else {
                    AddError(OverflowErrorMessage(i, "-", j), source);
                    return utils::Failure;
                }
            } else {
                using T = UnwrapNumber<NumberT>;
                auto subtract_values = [](T lhs, T rhs) {
                    if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                        // Ensure no UB for signed underflow
                        using UT = std::make_unsigned_t<T>;
                        return static_cast<T>(static_cast<UT>(lhs) - static_cast<UT>(rhs));
                    } else {
                        return lhs - rhs;
                    }
                };
                result = subtract_values(i.value, j.value);
            }
            return CreateElement(builder, c0->Type(), result);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpMultiply(const sem::Type* ty,
                                        utils::VectorRef<const sem::Constant*> args,
                                        const Source& source) {
    TINT_SCOPED_ASSIGNMENT(current_source, &source);
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        return Dispatch_fia_fiu32_f16(MulFunc(c0->Type()), c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpMultiplyMatVec(const sem::Type* ty,
                                              utils::VectorRef<const sem::Constant*> args,
                                              const Source& source) {
    TINT_SCOPED_ASSIGNMENT(current_source, &source);
    auto* mat_ty = args[0]->Type()->As<sem::Matrix>();
    auto* vec_ty = args[1]->Type()->As<sem::Vector>();
    auto* elem_ty = vec_ty->type();

    auto dot = [&](const sem::Constant* m, size_t row, const sem::Constant* v) {
        ImplResult result;
        switch (mat_ty->columns()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(elem_ty),        //
                                             m->Index(0)->Index(row),  //
                                             m->Index(1)->Index(row),  //
                                             v->Index(0),              //
                                             v->Index(1));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(elem_ty),        //
                                             m->Index(0)->Index(row),  //
                                             m->Index(1)->Index(row),  //
                                             m->Index(2)->Index(row),  //
                                             v->Index(0),              //
                                             v->Index(1), v->Index(2));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(elem_ty),        //
                                             m->Index(0)->Index(row),  //
                                             m->Index(1)->Index(row),  //
                                             m->Index(2)->Index(row),  //
                                             m->Index(3)->Index(row),  //
                                             v->Index(0),              //
                                             v->Index(1),              //
                                             v->Index(2),              //
                                             v->Index(3));
                break;
        }
        return result;
    };

    utils::Vector<const sem::Constant*, 4> result;
    for (size_t i = 0; i < mat_ty->rows(); ++i) {
        auto r = dot(args[0], i, args[1]);  // matrix row i * vector
        if (!r) {
            return utils::Failure;
        }
        result.Push(r.Get());
    }
    return CreateComposite(builder, ty, result);
}
ConstEval::Result ConstEval::OpMultiplyVecMat(const sem::Type* ty,
                                              utils::VectorRef<const sem::Constant*> args,
                                              const Source& source) {
    TINT_SCOPED_ASSIGNMENT(current_source, &source);
    auto* vec_ty = args[0]->Type()->As<sem::Vector>();
    auto* mat_ty = args[1]->Type()->As<sem::Matrix>();
    auto* elem_ty = vec_ty->type();

    auto dot = [&](const sem::Constant* v, const sem::Constant* m, size_t col) {
        ImplResult result;
        switch (mat_ty->rows()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(elem_ty),        //
                                             m->Index(col)->Index(0),  //
                                             m->Index(col)->Index(1),  //
                                             v->Index(0),              //
                                             v->Index(1));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(elem_ty),        //
                                             m->Index(col)->Index(0),  //
                                             m->Index(col)->Index(1),  //
                                             m->Index(col)->Index(2),
                                             v->Index(0),  //
                                             v->Index(1),  //
                                             v->Index(2));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(elem_ty),        //
                                             m->Index(col)->Index(0),  //
                                             m->Index(col)->Index(1),  //
                                             m->Index(col)->Index(2),  //
                                             m->Index(col)->Index(3),  //
                                             v->Index(0),              //
                                             v->Index(1),              //
                                             v->Index(2),              //
                                             v->Index(3));
        }
        return result;
    };

    utils::Vector<const sem::Constant*, 4> result;
    for (size_t i = 0; i < mat_ty->columns(); ++i) {
        auto r = dot(args[0], args[1], i);  // vector * matrix col i
        if (!r) {
            return utils::Failure;
        }
        result.Push(r.Get());
    }
    return CreateComposite(builder, ty, result);
}

ConstEval::Result ConstEval::OpMultiplyMatMat(const sem::Type* ty,
                                              utils::VectorRef<const sem::Constant*> args,
                                              const Source& source) {
    TINT_SCOPED_ASSIGNMENT(current_source, &source);
    auto* mat1 = args[0];
    auto* mat2 = args[1];
    auto* mat1_ty = mat1->Type()->As<sem::Matrix>();
    auto* mat2_ty = mat2->Type()->As<sem::Matrix>();
    auto* elem_ty = mat1_ty->type();

    auto dot = [&](const sem::Constant* m1, size_t row, const sem::Constant* m2, size_t col) {
        auto m1e = [&](size_t r, size_t c) { return m1->Index(c)->Index(r); };
        auto m2e = [&](size_t r, size_t c) { return m2->Index(c)->Index(r); };

        ImplResult result;
        switch (mat1_ty->columns()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(elem_ty),  //
                                             m1e(row, 0),        //
                                             m1e(row, 1),        //
                                             m2e(0, col),        //
                                             m2e(1, col));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(elem_ty),  //
                                             m1e(row, 0),        //
                                             m1e(row, 1),        //
                                             m1e(row, 2),        //
                                             m2e(0, col),        //
                                             m2e(1, col),        //
                                             m2e(2, col));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(elem_ty),  //
                                             m1e(row, 0),        //
                                             m1e(row, 1),        //
                                             m1e(row, 2),        //
                                             m1e(row, 3),        //
                                             m2e(0, col),        //
                                             m2e(1, col),        //
                                             m2e(2, col),        //
                                             m2e(3, col));
                break;
        }
        return result;
    };

    utils::Vector<const sem::Constant*, 4> result_mat;
    for (size_t c = 0; c < mat2_ty->columns(); ++c) {
        utils::Vector<const sem::Constant*, 4> col_vec;
        for (size_t r = 0; r < mat1_ty->rows(); ++r) {
            auto v = dot(mat1, r, mat2, c);  // mat1 row r * mat2 col c
            if (!v) {
                return utils::Failure;
            }
            col_vec.Push(v.Get());  // mat1 row r * mat2 col c
        }

        // Add column vector to matrix
        auto* col_vec_ty = ty->As<sem::Matrix>()->ColumnType();
        result_mat.Push(CreateComposite(builder, col_vec_ty, col_vec));
    }
    return CreateComposite(builder, ty, result_mat);
}

ConstEval::Result ConstEval::OpDivide(const sem::Type* ty,
                                      utils::VectorRef<const sem::Constant*> args,
                                      const Source& source) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            using NumberT = decltype(i);
            NumberT result;
            if constexpr (IsAbstract<NumberT>) {
                // Check for over/underflow for abstract values
                if (auto r = CheckedDiv(i, j)) {
                    result = r->value;
                } else {
                    AddError(OverflowErrorMessage(i, "/", j), source);
                    return utils::Failure;
                }
            } else {
                using T = UnwrapNumber<NumberT>;
                auto divide_values = [](T lhs, T rhs) {
                    if constexpr (std::is_integral_v<T>) {
                        // For integers, lhs / 0 returns lhs
                        if (rhs == 0) {
                            return lhs;
                        }

                        if constexpr (std::is_signed_v<T>) {
                            // For signed integers, for lhs / -1, return lhs if lhs is the
                            // most negative value
                            if (rhs == -1 && lhs == std::numeric_limits<T>::min()) {
                                return lhs;
                            }
                        }
                    }
                    return lhs / rhs;
                };
                result = divide_values(i.value, j.value);
            }
            return CreateElement(builder, c0->Type(), result);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpEqual(const sem::Type* ty,
                                     utils::VectorRef<const sem::Constant*> args,
                                     const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), i == j);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpNotEqual(const sem::Type* ty,
                                        utils::VectorRef<const sem::Constant*> args,
                                        const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), i != j);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpLessThan(const sem::Type* ty,
                                        utils::VectorRef<const sem::Constant*> args,
                                        const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), i < j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpGreaterThan(const sem::Type* ty,
                                           utils::VectorRef<const sem::Constant*> args,
                                           const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), i > j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpLessThanEqual(const sem::Type* ty,
                                             utils::VectorRef<const sem::Constant*> args,
                                             const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), i <= j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpGreaterThanEqual(const sem::Type* ty,
                                                utils::VectorRef<const sem::Constant*> args,
                                                const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), i >= j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpAnd(const sem::Type* ty,
                                   utils::VectorRef<const sem::Constant*> args,
                                   const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            using T = decltype(i);
            T result;
            if constexpr (std::is_same_v<T, bool>) {
                result = i && j;
            } else {  // integral
                result = i & j;
            }
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), result);
        };
        return Dispatch_ia_iu32_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpOr(const sem::Type* ty,
                                  utils::VectorRef<const sem::Constant*> args,
                                  const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> ImplResult {
            using T = decltype(i);
            T result;
            if constexpr (std::is_same_v<T, bool>) {
                result = i || j;
            } else {  // integral
                result = i | j;
            }
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), result);
        };
        return Dispatch_ia_iu32_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpXor(const sem::Type* ty,
                                   utils::VectorRef<const sem::Constant*> args,
                                   const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) -> const ImplConstant* {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), decltype(i){i ^ j});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    auto r = TransformElements(builder, ty, transform, args[0], args[1]);
    if (builder.Diagnostics().contains_errors()) {
        return utils::Failure;
    }
    return r;
}

ConstEval::Result ConstEval::OpShiftLeft(const sem::Type* ty,
                                         utils::VectorRef<const sem::Constant*> args,
                                         const Source& source) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto e1, auto e2) -> const ImplConstant* {
            using NumberT = decltype(e1);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr size_t bit_width = BitWidth<NumberT>;
            UT e1u = static_cast<UT>(e1);
            UT e2u = static_cast<UT>(e2);

            if constexpr (IsAbstract<NumberT>) {
                // NOTE: Concrete shift left requires an unsigned rhs, so this check only applies
                // for abstracts.
                if (e2 < 0) {
                    AddError("cannot shift left by a negative value", source);
                    return nullptr;
                }

                // The e2 + 1 most significant bits of e1 must have the same bit value, otherwise
                // sign change (overflow) would occur.
                // Check sign change only if e2 is less than bit width of e1. If e1 is larger
                // than bit width, we check for non-representable value below.
                if (e2u < bit_width) {
                    UT must_match_msb = e2u + 1;
                    UT mask = ~UT{0} << (bit_width - must_match_msb);
                    if ((e1u & mask) != 0 && (e1u & mask) != mask) {
                        AddError("shift left operation results in sign change", source);
                        return nullptr;
                    }
                } else {
                    // If shift value >= bit_width, then any non-zero value would overflow
                    if (e1 != 0) {
                        AddError(OverflowErrorMessage(e1, "<<", e2), source);
                        return nullptr;
                    }
                }
            } else {
                if (static_cast<size_t>(e2) >= bit_width) {
                    // At shader/pipeline-creation time, it is an error to shift by the bit width of
                    // the lhs or greater.
                    // NOTE: At runtime, we shift by e2 % (bit width of e1).
                    AddError(
                        "shift left value must be less than the bit width of the lhs, which is " +
                            std::to_string(bit_width),
                        source);
                    return nullptr;
                }

                // The e2 + 1 most significant bits of e1 must have the same bit value, otherwise
                // sign change (overflow) would occur.
                size_t must_match_msb = e2u + 1;
                UT mask = ~UT{0} << (bit_width - must_match_msb);
                if ((e1u & mask) != 0 && (e1u & mask) != mask) {
                    AddError("shift left operation results in sign change", source);
                    return nullptr;
                }
            }

            // Avoid UB by left shifting as unsigned value
            auto result = static_cast<T>(static_cast<UT>(e1) << e2);
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), NumberT{result});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    auto r = TransformElements(builder, ty, transform, args[0], args[1]);
    if (builder.Diagnostics().contains_errors()) {
        return utils::Failure;
    }
    return r;
}

ConstEval::Result ConstEval::atan2(const sem::Type* ty,
                                   utils::VectorRef<const sem::Constant*> args,
                                   const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto i, auto j) {
            return CreateElement(builder, c0->Type(), decltype(i)(std::atan2(i.value, j.value)));
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::clamp(const sem::Type* ty,
                                   utils::VectorRef<const sem::Constant*> args,
                                   const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1,
                         const sem::Constant* c2) {
        auto create = [&](auto e, auto low, auto high) {
            return CreateElement(builder, c0->Type(),
                                 decltype(e)(std::min(std::max(e, low), high)));
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1, c2);
    };
    return TransformElements(builder, ty, transform, args[0], args[1], args[2]);
}

ConstEval::Result ConstEval::select_bool(const sem::Type* ty,
                                         utils::VectorRef<const sem::Constant*> args,
                                         const Source&) {
    auto cond = args[2]->As<bool>();
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1) {
        auto create = [&](auto f, auto t) -> ImplResult {
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), cond ? t : f);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::select_boolvec(const sem::Type* ty,
                                            utils::VectorRef<const sem::Constant*> args,
                                            const Source&) {
    auto transform = [&](const sem::Constant* c0, const sem::Constant* c1, size_t index) {
        auto create = [&](auto f, auto t) -> ImplResult {
            // Get corresponding bool value at the current vector value index
            auto cond = args[2]->Index(index)->As<bool>();
            return CreateElement(builder, sem::Type::DeepestElementOf(ty), cond ? t : f);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::Convert(const sem::Type* target_ty,
                                     const sem::Constant* value,
                                     const Source& source) {
    if (value->Type() == target_ty) {
        return value;
    }
    return static_cast<const ImplConstant*>(value)->Convert(builder, target_ty, source);
}

void ConstEval::AddError(const std::string& msg, const Source& source) const {
    builder.Diagnostics().add_error(diag::System::Resolver, msg, source);
}

void ConstEval::AddWarning(const std::string& msg, const Source& source) const {
    builder.Diagnostics().add_warning(diag::System::Resolver, msg, source);
}

}  // namespace tint::resolver
