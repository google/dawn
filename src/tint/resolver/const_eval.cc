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
#include <iomanip>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "src/tint/constant/composite.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/constant/splat.h"
#include "src/tint/constant/value.h"
#include "src/tint/number.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/switch.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/array.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/struct.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/utils/bitcast.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/string_stream.h"
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
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_iu32(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::U32*) { return f(cs->template ValueAs<u32>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fiu32(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::U32*) { return f(cs->template ValueAs<u32>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_ia_iu32(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::U32*) { return f(cs->template ValueAs<u32>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_ia_iu32_bool(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::U32*) { return f(cs->template ValueAs<u32>()...); },
        [&](const type::Bool*) { return f(cs->template ValueAs<bool>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fi32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::F16*) { return f(cs->template ValueAs<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fiu32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::U32*) { return f(cs->template ValueAs<u32>()...); },
        [&](const type::F16*) { return f(cs->template ValueAs<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fiu32_f16_bool(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const type::U32*) { return f(cs->template ValueAs<u32>()...); },
        [&](const type::F16*) { return f(cs->template ValueAs<f16>()...); },
        [&](const type::Bool*) { return f(cs->template ValueAs<bool>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fa_f32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const type::F16*) { return f(cs->template ValueAs<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_bool(F&& f, CONSTANTS&&... cs) {
    return f(cs->template ValueAs<bool>()...);
}

/// ZeroTypeDispatch is a helper for calling the function `f`, passing a single zero-value argument
/// of the C++ type that corresponds to the type::Type `type`. For example, calling
/// `ZeroTypeDispatch()` with a type of `type::I32*` will call the function f with a single argument
/// of `i32(0)`.
/// @returns the value returned by calling `f`.
/// @note `type` must be a scalar or abstract numeric type. Other types will not call `f`, and will
/// return the zero-initialized value of the return type for `f`.
template <typename F>
auto ZeroTypeDispatch(const type::Type* type, F&& f) {
    return Switch(
        type,                                                      //
        [&](const type::AbstractInt*) { return f(AInt(0)); },      //
        [&](const type::AbstractFloat*) { return f(AFloat(0)); },  //
        [&](const type::I32*) { return f(i32(0)); },               //
        [&](const type::U32*) { return f(u32(0)); },               //
        [&](const type::F32*) { return f(f32(0)); },               //
        [&](const type::F16*) { return f(f16(0)); },               //
        [&](const type::Bool*) { return f(static_cast<bool>(0)); });
}

template <typename NumberT>
std::string OverflowErrorMessage(NumberT lhs, const char* op, NumberT rhs) {
    utils::StringStream ss;
    ss << "'" << lhs.value << " " << op << " " << rhs.value << "' cannot be represented as '"
       << FriendlyName<NumberT>() << "'";
    return ss.str();
}

template <typename VALUE_TY>
std::string OverflowErrorMessage(VALUE_TY value, std::string_view target_ty) {
    utils::StringStream ss;
    ss << "value " << value << " cannot be represented as "
       << "'" << target_ty << "'";
    return ss.str();
}

template <typename NumberT>
std::string OverflowExpErrorMessage(std::string_view base, NumberT exp) {
    utils::StringStream ss;
    ss << base << "^" << exp << " cannot be represented as "
       << "'" << FriendlyName<NumberT>() << "'";
    return ss.str();
}

/// @returns the number of consecutive leading bits in `@p e` set to `@p bit_value_to_count`.
template <typename T>
std::make_unsigned_t<T> CountLeadingBits(T e, T bit_value_to_count) {
    using UT = std::make_unsigned_t<T>;
    constexpr UT kNumBits = sizeof(UT) * 8;
    constexpr UT kLeftMost = UT{1} << (kNumBits - 1);
    const UT b = bit_value_to_count == 0 ? UT{0} : kLeftMost;

    auto v = static_cast<UT>(e);
    auto count = UT{0};
    while ((count < kNumBits) && ((v & kLeftMost) == b)) {
        ++count;
        v <<= 1;
    }
    return count;
}

/// @returns the number of consecutive trailing bits set to `@p bit_value_to_count` in `@p e`
template <typename T>
std::make_unsigned_t<T> CountTrailingBits(T e, T bit_value_to_count) {
    using UT = std::make_unsigned_t<T>;
    constexpr UT kNumBits = sizeof(UT) * 8;
    constexpr UT kRightMost = UT{1};
    const UT b = static_cast<UT>(bit_value_to_count);

    auto v = static_cast<UT>(e);
    auto count = UT{0};
    while ((count < kNumBits) && ((v & kRightMost) == b)) {
        ++count;
        v >>= 1;
    }
    return count;
}

template <typename T>
ConstEval::Result ScalarConvert(const constant::Scalar<T>* scalar,
                                ProgramBuilder& builder,
                                const type::Type* target_ty,
                                const Source& source,
                                bool use_runtime_semantics) {
    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);
    if (target_ty == scalar->type) {
        // If the types are identical, then no conversion is needed.
        return scalar;
    }
    return ZeroTypeDispatch(target_ty, [&](auto zero_to) -> ConstEval::Result {
        // `value` is the source value.
        // `FROM` is the source type.
        // `TO` is the target type.
        using TO = std::decay_t<decltype(zero_to)>;
        using FROM = T;
        if constexpr (std::is_same_v<TO, bool>) {
            // [x -> bool]
            return builder.create<constant::Scalar<TO>>(target_ty, !scalar->IsPositiveZero());
        } else if constexpr (std::is_same_v<FROM, bool>) {
            // [bool -> x]
            return builder.create<constant::Scalar<TO>>(target_ty, TO(scalar->value ? 1 : 0));
        } else if (auto conv = CheckedConvert<TO>(scalar->value)) {
            // Conversion success
            return builder.create<constant::Scalar<TO>>(target_ty, conv.Get());
            // --- Below this point are the failure cases ---
        } else if constexpr (IsAbstract<FROM>) {
            // [abstract-numeric -> x] - materialization failure
            auto msg = OverflowErrorMessage(scalar->value, target_ty->FriendlyName());
            if (use_runtime_semantics) {
                builder.Diagnostics().add_warning(tint::diag::System::Resolver, msg, source);
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<constant::Scalar<TO>>(target_ty, TO::Lowest());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<constant::Scalar<TO>>(target_ty, TO::Highest());
                }
            } else {
                builder.Diagnostics().add_error(tint::diag::System::Resolver, msg, source);
                return utils::Failure;
            }
        } else if constexpr (IsFloatingPoint<TO>) {
            // [x -> floating-point] - number not exactly representable
            // https://www.w3.org/TR/WGSL/#floating-point-conversion
            auto msg = OverflowErrorMessage(scalar->value, target_ty->FriendlyName());
            if (use_runtime_semantics) {
                builder.Diagnostics().add_warning(tint::diag::System::Resolver, msg, source);
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<constant::Scalar<TO>>(target_ty, TO::Lowest());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<constant::Scalar<TO>>(target_ty, TO::Highest());
                }
            } else {
                builder.Diagnostics().add_error(tint::diag::System::Resolver, msg, source);
                return utils::Failure;
            }
        } else if constexpr (IsFloatingPoint<FROM>) {
            // [floating-point -> integer] - number not exactly representable
            // https://www.w3.org/TR/WGSL/#floating-point-conversion
            switch (conv.Failure()) {
                case ConversionFailure::kExceedsNegativeLimit:
                    return builder.create<constant::Scalar<TO>>(target_ty, TO::Lowest());
                case ConversionFailure::kExceedsPositiveLimit:
                    return builder.create<constant::Scalar<TO>>(target_ty, TO::Highest());
            }
        } else if constexpr (IsIntegral<FROM>) {
            // [integer -> integer] - number not exactly representable
            // Static cast
            return builder.create<constant::Scalar<TO>>(target_ty, static_cast<TO>(scalar->value));
        }
        return nullptr;  // Expression is not constant.
    });
    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
}

// Forward declare
ConstEval::Result ConvertInternal(const constant::Value* c,
                                  ProgramBuilder& builder,
                                  const type::Type* target_ty,
                                  const Source& source,
                                  bool use_runtime_semantics);

ConstEval::Result CompositeConvert(const constant::Value* value,
                                   ProgramBuilder& builder,
                                   const type::Type* target_ty,
                                   const Source& source,
                                   bool use_runtime_semantics) {
    const size_t el_count = value->NumElements();

    // Convert each of the composite element types.
    utils::Vector<const constant::Value*, 4> conv_els;
    conv_els.Reserve(el_count);

    std::function<const type::Type*(size_t idx)> target_el_ty;
    if (auto* str = target_ty->As<type::Struct>()) {
        if (TINT_UNLIKELY(str->Members().Length() != el_count)) {
            TINT_ICE(Resolver, builder.Diagnostics())
                << "const-eval conversion of structure has mismatched element counts";
            return utils::Failure;
        }
        target_el_ty = [str](size_t idx) { return str->Members()[idx]->Type(); };
    } else {
        auto* el_ty = type::Type::ElementOf(target_ty);
        target_el_ty = [el_ty](size_t) { return el_ty; };
    }

    for (size_t i = 0; i < el_count; i++) {
        auto* el = value->Index(i);
        auto conv_el = ConvertInternal(el, builder, target_el_ty(conv_els.Length()), source,
                                       use_runtime_semantics);
        if (!conv_el) {
            return utils::Failure;
        }
        if (!conv_el.Get()) {
            return nullptr;
        }
        conv_els.Push(conv_el.Get());
    }
    return builder.create<constant::Composite>(target_ty, std::move(conv_els));
}

ConstEval::Result SplatConvert(const constant::Splat* splat,
                               ProgramBuilder& builder,
                               const type::Type* target_ty,
                               const Source& source,
                               bool use_runtime_semantics) {
    const type::Type* target_el_ty = nullptr;
    if (auto* str = target_ty->As<type::Struct>()) {
        // Structure conversion.
        auto members = str->Members();
        target_el_ty = members[0]->Type();

        // Structures can only be converted during materialization. The user cannot declare the
        // target structure type, so each member type must be the same default materialization type.
        for (size_t i = 1; i < members.Length(); i++) {
            if (members[i]->Type() != target_el_ty) {
                TINT_ICE(Resolver, builder.Diagnostics())
                    << "inconsistent target struct member types for SplatConvert";
                return utils::Failure;
            }
        }
    } else {
        target_el_ty = type::Type::ElementOf(target_ty);
    }
    // Convert the single splatted element type.
    auto conv_el = ConvertInternal(splat->el, builder, target_el_ty, source, use_runtime_semantics);
    if (!conv_el) {
        return utils::Failure;
    }
    if (!conv_el.Get()) {
        return nullptr;
    }
    return builder.create<constant::Splat>(target_ty, conv_el.Get(), splat->count);
}

ConstEval::Result ConvertInternal(const constant::Value* c,
                                  ProgramBuilder& builder,
                                  const type::Type* target_ty,
                                  const Source& source,
                                  bool use_runtime_semantics) {
    return Switch(
        c,
        [&](const constant::Scalar<tint::AFloat>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Scalar<tint::AInt>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Scalar<tint::u32>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Scalar<tint::i32>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Scalar<tint::f32>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Scalar<tint::f16>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Scalar<bool>* val) {
            return ScalarConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Splat* val) {
            return SplatConvert(val, builder, target_ty, source, use_runtime_semantics);
        },
        [&](const constant::Composite* val) {
            return CompositeConvert(val, builder, target_ty, source, use_runtime_semantics);
        });
}

namespace detail {
/// Implementation of TransformElements
template <typename F, typename... CONSTANTS>
ConstEval::Result TransformElements(ProgramBuilder& builder,
                                    const type::Type* composite_ty,
                                    F&& f,
                                    size_t index,
                                    CONSTANTS&&... cs) {
    uint32_t n = 0;
    auto* ty = First(cs...)->Type();
    auto* el_ty = type::Type::ElementOf(ty, &n);
    if (el_ty == ty) {
        constexpr bool kHasIndexParam =
            utils::traits::IsType<size_t, utils::traits::LastParameterType<F>>;
        if constexpr (kHasIndexParam) {
            return f(cs..., index);
        } else {
            return f(cs...);
        }
    }
    utils::Vector<const constant::Value*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        if (auto el = detail::TransformElements(builder, type::Type::ElementOf(composite_ty),
                                                std::forward<F>(f), index + i, cs->Index(i)...)) {
            els.Push(el.Get());

        } else {
            return el.Failure();
        }
    }
    return builder.create<constant::Composite>(composite_ty, std::move(els));
}
}  // namespace detail

/// TransformElements constructs a new constant of type `composite_ty` by applying the
/// transformation function `f` on each of the most deeply nested elements of 'cs'. Assumes that all
/// input constants `cs` are of the same arity (all scalars or all vectors of the same size).
/// If `f`'s last argument is a `size_t`, then the index of the most deeply nested element inside
/// the most deeply nested aggregate type will be passed in.
template <typename F, typename... CONSTANTS>
ConstEval::Result TransformElements(ProgramBuilder& builder,
                                    const type::Type* composite_ty,
                                    F&& f,
                                    CONSTANTS&&... cs) {
    return detail::TransformElements(builder, composite_ty, f, 0, cs...);
}

/// TransformBinaryElements constructs a new constant of type `composite_ty` by applying the
/// transformation function 'f' on each of the most deeply nested elements of both `c0` and `c1`.
/// Unlike TransformElements, this function handles the constants being of different arity, e.g.
/// vector-scalar, scalar-vector.
template <typename F>
ConstEval::Result TransformBinaryElements(ProgramBuilder& builder,
                                          const type::Type* composite_ty,
                                          F&& f,
                                          const constant::Value* c0,
                                          const constant::Value* c1) {
    uint32_t n0 = 0;
    type::Type::ElementOf(c0->Type(), &n0);
    uint32_t n1 = 0;
    type::Type::ElementOf(c1->Type(), &n1);
    uint32_t max_n = std::max(n0, n1);
    // If arity of both constants is 1, invoke callback
    if (max_n == 1u) {
        return f(c0, c1);
    }

    utils::Vector<const constant::Value*, 8> els;
    els.Reserve(max_n);
    for (uint32_t i = 0; i < max_n; i++) {
        auto nested_or_self = [&](auto* c, uint32_t num_elems) {
            if (num_elems == 1) {
                return c;
            }
            return c->Index(i);
        };
        if (auto el = TransformBinaryElements(builder, type::Type::ElementOf(composite_ty),
                                              std::forward<F>(f), nested_or_self(c0, n0),
                                              nested_or_self(c1, n1))) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return builder.create<constant::Composite>(composite_ty, std::move(els));
}
}  // namespace

ConstEval::ConstEval(ProgramBuilder& b, bool use_runtime_semantics /* = false */)
    : builder(b), use_runtime_semantics_(use_runtime_semantics) {}

template <typename T>
ConstEval::Result ConstEval::CreateScalar(const Source& source, const type::Type* t, T v) {
    static_assert(IsNumber<T> || std::is_same_v<T, bool>, "T must be a Number or bool");
    TINT_ASSERT(Resolver, t->is_scalar());

    if constexpr (IsFloatingPoint<T>) {
        if (!std::isfinite(v.value)) {
            AddError(OverflowErrorMessage(v, t->FriendlyName()), source);
            if (use_runtime_semantics_) {
                return ZeroValue(t);
            } else {
                return utils::Failure;
            }
        }
    }
    return builder.create<constant::Scalar<T>>(t, v);
}

const constant::Value* ConstEval::ZeroValue(const type::Type* type) {
    return Switch(
        type,  //
        [&](const type::Vector* v) -> const constant::Value* {
            auto* zero_el = ZeroValue(v->type());
            return builder.create<constant::Splat>(type, zero_el, v->Width());
        },
        [&](const type::Matrix* m) -> const constant::Value* {
            auto* zero_el = ZeroValue(m->ColumnType());
            return builder.create<constant::Splat>(type, zero_el, m->columns());
        },
        [&](const type::Array* a) -> const constant::Value* {
            if (auto n = a->ConstantCount()) {
                if (auto* zero_el = ZeroValue(a->ElemType())) {
                    return builder.create<constant::Splat>(type, zero_el, n.value());
                }
            }
            return nullptr;
        },
        [&](const type::Struct* s) -> const constant::Value* {
            utils::Hashmap<const type::Type*, const constant::Value*, 8> zero_by_type;
            utils::Vector<const constant::Value*, 4> zeros;
            zeros.Reserve(s->Members().Length());
            for (auto* member : s->Members()) {
                auto* zero = zero_by_type.GetOrCreate(member->Type(),
                                                      [&] { return ZeroValue(member->Type()); });
                if (!zero) {
                    return nullptr;
                }
                zeros.Push(zero);
            }
            if (zero_by_type.Count() == 1) {
                // All members were of the same type, so the zero value is the same for all members.
                return builder.create<constant::Splat>(type, zeros[0], s->Members().Length());
            }
            return builder.create<constant::Composite>(s, std::move(zeros));
        },
        [&](Default) -> const constant::Value* {
            return ZeroTypeDispatch(type, [&](auto zero) -> const constant::Value* {
                auto el = CreateScalar(Source{}, type, zero);
                TINT_ASSERT(Resolver, el);
                return el.Get();
            });
        });
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Add(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedAdd(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "+", b), source);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return utils::Failure;
            }
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
utils::Result<NumberT> ConstEval::Sub(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedSub(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "-", b), source);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return utils::Failure;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto sub_values = [](T lhs, T rhs) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // Ensure no UB for signed overflow
                using UT = std::make_unsigned_t<T>;
                return static_cast<T>(static_cast<UT>(lhs) - static_cast<UT>(rhs));
            } else {
                return lhs - rhs;
            }
        };
        result = sub_values(a.value, b.value);
    }
    return result;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Mul(const Source& source, NumberT a, NumberT b) {
    using T = UnwrapNumber<NumberT>;
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedMul(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "*", b), source);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return utils::Failure;
            }
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
utils::Result<NumberT> ConstEval::Div(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedDiv(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "/", b), source);
            if (use_runtime_semantics_) {
                return a;
            } else {
                return utils::Failure;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto lhs = a.value;
        auto rhs = b.value;
        if (rhs == 0) {
            // For integers (as for floats), lhs / 0 is an error
            AddError(OverflowErrorMessage(a, "/", b), source);
            if (use_runtime_semantics_) {
                return a;
            } else {
                return utils::Failure;
            }
        }
        if constexpr (std::is_signed_v<T>) {
            // For signed integers, lhs / -1 where lhs is the
            // most negative value is an error
            if (rhs == -1 && lhs == std::numeric_limits<T>::min()) {
                AddError(OverflowErrorMessage(a, "/", b), source);
                if (use_runtime_semantics_) {
                    return a;
                } else {
                    return utils::Failure;
                }
            }
        }
        result = lhs / rhs;
    }
    return result;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Mod(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedMod(a, b)) {
            result = r->value;
        } else {
            AddError(OverflowErrorMessage(a, "%", b), source);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return utils::Failure;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto lhs = a.value;
        auto rhs = b.value;
        if (rhs == 0) {
            // lhs % 0 is an error
            AddError(OverflowErrorMessage(a, "%", b), source);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return utils::Failure;
            }
        }
        if constexpr (std::is_signed_v<T>) {
            // For signed integers, lhs % -1 where lhs is the
            // most negative value is an error
            if (rhs == -1 && lhs == std::numeric_limits<T>::min()) {
                AddError(OverflowErrorMessage(a, "%", b), source);
                if (use_runtime_semantics_) {
                    return NumberT{0};
                } else {
                    return utils::Failure;
                }
            }
        }
        result = lhs % rhs;
    }
    return result;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Dot2(const Source& source,
                                       NumberT a1,
                                       NumberT a2,
                                       NumberT b1,
                                       NumberT b2) {
    auto r1 = Mul(source, a1, b1);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(source, a2, b2);
    if (!r2) {
        return utils::Failure;
    }
    auto r = Add(source, r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Dot3(const Source& source,
                                       NumberT a1,
                                       NumberT a2,
                                       NumberT a3,
                                       NumberT b1,
                                       NumberT b2,
                                       NumberT b3) {
    auto r1 = Mul(source, a1, b1);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(source, a2, b2);
    if (!r2) {
        return utils::Failure;
    }
    auto r3 = Mul(source, a3, b3);
    if (!r3) {
        return utils::Failure;
    }
    auto r = Add(source, r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(source, r.Get(), r3.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Dot4(const Source& source,
                                       NumberT a1,
                                       NumberT a2,
                                       NumberT a3,
                                       NumberT a4,
                                       NumberT b1,
                                       NumberT b2,
                                       NumberT b3,
                                       NumberT b4) {
    auto r1 = Mul(source, a1, b1);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(source, a2, b2);
    if (!r2) {
        return utils::Failure;
    }
    auto r3 = Mul(source, a3, b3);
    if (!r3) {
        return utils::Failure;
    }
    auto r4 = Mul(source, a4, b4);
    if (!r4) {
        return utils::Failure;
    }
    auto r = Add(source, r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(source, r.Get(), r3.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(source, r.Get(), r4.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Det2(const Source& source,
                                       NumberT a,
                                       NumberT b,
                                       NumberT c,
                                       NumberT d) {
    // | a c |
    // | b d |
    //
    // =
    //
    // a * d - c * b

    auto r1 = Mul(source, a, d);
    if (!r1) {
        return utils::Failure;
    }
    auto r2 = Mul(source, c, b);
    if (!r2) {
        return utils::Failure;
    }
    auto r = Sub(source, r1.Get(), r2.Get());
    if (!r) {
        return utils::Failure;
    }
    return r;
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Det3(const Source& source,
                                       NumberT a,
                                       NumberT b,
                                       NumberT c,
                                       NumberT d,
                                       NumberT e,
                                       NumberT f,
                                       NumberT g,
                                       NumberT h,
                                       NumberT i) {
    // | a d g |
    // | b e h |
    // | c f i |
    //
    // =
    //
    // a | e h | - d | b h | + g | b e |
    //   | f i |     | c i |     | c f |

    auto det1 = Det2(source, e, f, h, i);
    if (!det1) {
        return utils::Failure;
    }
    auto a_det1 = Mul(source, a, det1.Get());
    if (!a_det1) {
        return utils::Failure;
    }
    auto det2 = Det2(source, b, c, h, i);
    if (!det2) {
        return utils::Failure;
    }
    auto d_det2 = Mul(source, d, det2.Get());
    if (!d_det2) {
        return utils::Failure;
    }
    auto det3 = Det2(source, b, c, e, f);
    if (!det3) {
        return utils::Failure;
    }
    auto g_det3 = Mul(source, g, det3.Get());
    if (!g_det3) {
        return utils::Failure;
    }
    auto r = Sub(source, a_det1.Get(), d_det2.Get());
    if (!r) {
        return utils::Failure;
    }
    return Add(source, r.Get(), g_det3.Get());
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Det4(const Source& source,
                                       NumberT a,
                                       NumberT b,
                                       NumberT c,
                                       NumberT d,
                                       NumberT e,
                                       NumberT f,
                                       NumberT g,
                                       NumberT h,
                                       NumberT i,
                                       NumberT j,
                                       NumberT k,
                                       NumberT l,
                                       NumberT m,
                                       NumberT n,
                                       NumberT o,
                                       NumberT p) {
    // | a e i m |
    // | b f j n |
    // | c g k o |
    // | d h l p |
    //
    // =
    //
    // a | f j n | - e | b j n | + i | b f n | - m | b f j |
    //   | g k o |     | c k o |     | c g o |     | c g k |
    //   | h l p |     | d l p |     | d h p |     | d h l |

    auto det1 = Det3(source, f, g, h, j, k, l, n, o, p);
    if (!det1) {
        return utils::Failure;
    }
    auto a_det1 = Mul(source, a, det1.Get());
    if (!a_det1) {
        return utils::Failure;
    }
    auto det2 = Det3(source, b, c, d, j, k, l, n, o, p);
    if (!det2) {
        return utils::Failure;
    }
    auto e_det2 = Mul(source, e, det2.Get());
    if (!e_det2) {
        return utils::Failure;
    }
    auto det3 = Det3(source, b, c, d, f, g, h, n, o, p);
    if (!det3) {
        return utils::Failure;
    }
    auto i_det3 = Mul(source, i, det3.Get());
    if (!i_det3) {
        return utils::Failure;
    }
    auto det4 = Det3(source, b, c, d, f, g, h, j, k, l);
    if (!det4) {
        return utils::Failure;
    }
    auto m_det4 = Mul(source, m, det4.Get());
    if (!m_det4) {
        return utils::Failure;
    }
    auto r = Sub(source, a_det1.Get(), e_det2.Get());
    if (!r) {
        return utils::Failure;
    }
    r = Add(source, r.Get(), i_det3.Get());
    if (!r) {
        return utils::Failure;
    }
    return Sub(source, r.Get(), m_det4.Get());
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Sqrt(const Source& source, NumberT v) {
    if (v < NumberT(0)) {
        AddError("sqrt must be called with a value >= 0", source);
        if (use_runtime_semantics_) {
            return NumberT{0};
        } else {
            return utils::Failure;
        }
    }
    return NumberT{std::sqrt(v)};
}

auto ConstEval::SqrtFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto v) -> ConstEval::Result {
        if (auto r = Sqrt(source, v)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

template <typename NumberT>
utils::Result<NumberT> ConstEval::Clamp(const Source&, NumberT e, NumberT low, NumberT high) {
    return NumberT{std::min(std::max(e, low), high)};
}

auto ConstEval::ClampFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto e, auto low, auto high) -> ConstEval::Result {
        if (auto r = Clamp(source, e, low, high)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::AddFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ConstEval::Result {
        if (auto r = Add(source, a1, a2)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::SubFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ConstEval::Result {
        if (auto r = Sub(source, a1, a2)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::MulFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ConstEval::Result {
        if (auto r = Mul(source, a1, a2)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::DivFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ConstEval::Result {
        if (auto r = Div(source, a1, a2)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::ModFunc(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2) -> ConstEval::Result {
        if (auto r = Mod(source, a1, a2)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Dot2Func(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2, auto b1, auto b2) -> ConstEval::Result {
        if (auto r = Dot2(source, a1, a2, b1, b2)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Dot3Func(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2, auto a3, auto b1, auto b2, auto b3) -> ConstEval::Result {
        if (auto r = Dot3(source, a1, a2, a3, b1, b2, b3)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Dot4Func(const Source& source, const type::Type* elem_ty) {
    return [=](auto a1, auto a2, auto a3, auto a4, auto b1, auto b2, auto b3,
               auto b4) -> ConstEval::Result {
        if (auto r = Dot4(source, a1, a2, a3, a4, b1, b2, b3, b4)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

ConstEval::Result ConstEval::Dot(const Source& source,
                                 const constant::Value* v1,
                                 const constant::Value* v2) {
    auto* vec_ty = v1->Type()->As<type::Vector>();
    TINT_ASSERT(Resolver, vec_ty);
    auto* elem_ty = vec_ty->type();
    switch (vec_ty->Width()) {
        case 2:
            return Dispatch_fia_fiu32_f16(   //
                Dot2Func(source, elem_ty),   //
                v1->Index(0), v1->Index(1),  //
                v2->Index(0), v2->Index(1));
        case 3:
            return Dispatch_fia_fiu32_f16(                 //
                Dot3Func(source, elem_ty),                 //
                v1->Index(0), v1->Index(1), v1->Index(2),  //
                v2->Index(0), v2->Index(1), v2->Index(2));
        case 4:
            return Dispatch_fia_fiu32_f16(                               //
                Dot4Func(source, elem_ty),                               //
                v1->Index(0), v1->Index(1), v1->Index(2), v1->Index(3),  //
                v2->Index(0), v2->Index(1), v2->Index(2), v2->Index(3));
    }
    TINT_ICE(Resolver, builder.Diagnostics()) << "Expected vector";
    return utils::Failure;
}

ConstEval::Result ConstEval::Length(const Source& source,
                                    const type::Type* ty,
                                    const constant::Value* c0) {
    auto* vec_ty = c0->Type()->As<type::Vector>();
    // Evaluates to the absolute value of e if T is scalar.
    if (vec_ty == nullptr) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            return CreateScalar(source, ty, NumberT{std::abs(e)});
        };
        return Dispatch_fa_f32_f16(create, c0);
    }

    // Evaluates to sqrt(e[0]^2 + e[1]^2 + ...) if T is a vector type.
    auto d = Dot(source, c0, c0);
    if (!d) {
        return utils::Failure;
    }
    return Dispatch_fa_f32_f16(SqrtFunc(source, ty), d.Get());
}

ConstEval::Result ConstEval::Mul(const Source& source,
                                 const type::Type* ty,
                                 const constant::Value* v1,
                                 const constant::Value* v2) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        return Dispatch_fia_fiu32_f16(MulFunc(source, c0->Type()), c0, c1);
    };
    return TransformBinaryElements(builder, ty, transform, v1, v2);
}

ConstEval::Result ConstEval::Sub(const Source& source,
                                 const type::Type* ty,
                                 const constant::Value* v1,
                                 const constant::Value* v2) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        return Dispatch_fia_fiu32_f16(SubFunc(source, c0->Type()), c0, c1);
    };
    return TransformBinaryElements(builder, ty, transform, v1, v2);
}

auto ConstEval::Det2Func(const Source& source, const type::Type* elem_ty) {
    return [=](auto a, auto b, auto c, auto d) -> ConstEval::Result {
        if (auto r = Det2(source, a, b, c, d)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Det3Func(const Source& source, const type::Type* elem_ty) {
    return [=](auto a, auto b, auto c, auto d, auto e, auto f, auto g, auto h,
               auto i) -> ConstEval::Result {
        if (auto r = Det3(source, a, b, c, d, e, f, g, h, i)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

auto ConstEval::Det4Func(const Source& source, const type::Type* elem_ty) {
    return [=](auto a, auto b, auto c, auto d, auto e, auto f, auto g, auto h, auto i, auto j,
               auto k, auto l, auto m, auto n, auto o, auto p) -> ConstEval::Result {
        if (auto r = Det4(source, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return utils::Failure;
    };
}

ConstEval::Result ConstEval::Literal(const type::Type* ty, const ast::LiteralExpression* literal) {
    auto& source = literal->source;
    return Switch(
        literal,
        [&](const ast::BoolLiteralExpression* lit) { return CreateScalar(source, ty, lit->value); },
        [&](const ast::IntLiteralExpression* lit) -> ConstEval::Result {
            switch (lit->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                    return CreateScalar(source, ty, AInt(lit->value));
                case ast::IntLiteralExpression::Suffix::kI:
                    return CreateScalar(source, ty, i32(lit->value));
                case ast::IntLiteralExpression::Suffix::kU:
                    return CreateScalar(source, ty, u32(lit->value));
            }
            return nullptr;
        },
        [&](const ast::FloatLiteralExpression* lit) -> ConstEval::Result {
            switch (lit->suffix) {
                case ast::FloatLiteralExpression::Suffix::kNone:
                    return CreateScalar(source, ty, AFloat(lit->value));
                case ast::FloatLiteralExpression::Suffix::kF:
                    return CreateScalar(source, ty, f32(lit->value));
                case ast::FloatLiteralExpression::Suffix::kH:
                    return CreateScalar(source, ty, f16(lit->value));
            }
            return nullptr;
        });
}

ConstEval::Result ConstEval::ArrayOrStructCtor(const type::Type* ty,
                                               utils::VectorRef<const constant::Value*> args) {
    if (args.IsEmpty()) {
        return ZeroValue(ty);
    }

    if (args.Length() == 1 && args[0]->Type() == ty) {
        // Identity constructor.
        return args[0];
    }

    // Multiple arguments. Must be a value constructor.
    return builder.create<constant::Composite>(ty, std::move(args));
}

ConstEval::Result ConstEval::Conv(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    uint32_t el_count = 0;
    auto* el_ty = type::Type::ElementOf(ty, &el_count);
    if (!el_ty) {
        return nullptr;
    }

    if (!args[0]) {
        return nullptr;  // Single argument is not constant.
    }

    return Convert(ty, args[0], source);
}

ConstEval::Result ConstEval::Zero(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*>,
                                  const Source&) {
    return ZeroValue(ty);
}

ConstEval::Result ConstEval::Identity(const type::Type*,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source&) {
    return args[0];
}

ConstEval::Result ConstEval::VecSplat(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source&) {
    if (auto* arg = args[0]) {
        return builder.create<constant::Splat>(ty, arg,
                                               static_cast<const type::Vector*>(ty)->Width());
    }
    return nullptr;
}

ConstEval::Result ConstEval::VecInitS(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source&) {
    return builder.create<constant::Composite>(ty, args);
}

ConstEval::Result ConstEval::VecInitM(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source&) {
    utils::Vector<const constant::Value*, 4> els;
    for (auto* arg : args) {
        auto* val = arg;
        if (!val) {
            return nullptr;
        }
        auto* arg_ty = arg->Type();
        if (auto* arg_vec = arg_ty->As<type::Vector>()) {
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
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::MatInitS(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source&) {
    auto* m = static_cast<const type::Matrix*>(ty);

    utils::Vector<const constant::Value*, 4> els;
    for (uint32_t c = 0; c < m->columns(); c++) {
        utils::Vector<const constant::Value*, 4> column;
        for (uint32_t r = 0; r < m->rows(); r++) {
            auto i = r + c * m->rows();
            column.Push(args[i]);
        }
        els.Push(builder.create<constant::Composite>(m->ColumnType(), std::move(column)));
    }
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::MatInitV(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source&) {
    return builder.create<constant::Composite>(ty, args);
}

ConstEval::Result ConstEval::Index(const type::Type* ty,
                                   const sem::ValueExpression* obj_expr,
                                   const sem::ValueExpression* idx_expr) {
    auto idx_val = idx_expr->ConstantValue();
    if (!idx_val) {
        return nullptr;
    }

    uint32_t el_count = 0;
    type::Type::ElementOf(obj_expr->Type()->UnwrapRef(), &el_count);

    AInt idx = idx_val->ValueAs<AInt>();
    if (idx < 0 || (el_count > 0 && idx >= el_count)) {
        std::string range;
        if (el_count > 0) {
            range = " [0.." + std::to_string(el_count - 1) + "]";
        }
        AddError("index " + std::to_string(idx) + " out of bounds" + range,
                 idx_expr->Declaration()->source);
        if (use_runtime_semantics_) {
            return ZeroValue(ty);
        } else {
            return utils::Failure;
        }
    }

    auto obj_val = obj_expr->ConstantValue();
    if (!obj_val) {
        return nullptr;
    }

    return obj_val->Index(static_cast<size_t>(idx));
}

ConstEval::Result ConstEval::MemberAccess(const sem::ValueExpression* obj_expr,
                                          const type::StructMember* member) {
    auto obj_val = obj_expr->ConstantValue();
    if (!obj_val) {
        return nullptr;
    }
    return obj_val->Index(static_cast<size_t>(member->Index()));
}

ConstEval::Result ConstEval::Swizzle(const type::Type* ty,
                                     const sem::ValueExpression* vec_expr,
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
    return builder.create<constant::Composite>(ty, std::move(values));
}

ConstEval::Result ConstEval::Bitcast(const type::Type* ty,
                                     const constant::Value* value,
                                     const Source& source) {
    auto* el_ty = type::Type::DeepestElementOf(ty);
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            return Switch(
                el_ty,
                [&](const type::U32*) {  //
                    auto r = utils::Bitcast<u32>(e);
                    return CreateScalar(source, el_ty, r);
                },
                [&](const type::I32*) {  //
                    auto r = utils::Bitcast<i32>(e);
                    return CreateScalar(source, el_ty, r);
                },
                [&](const type::F32*) {  //
                    auto r = utils::Bitcast<f32>(e);
                    return CreateScalar(source, el_ty, r);
                });
        };
        return Dispatch_fiu32(create, c0);
    };
    return TransformElements(builder, ty, transform, value);
}

ConstEval::Result ConstEval::OpComplement(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    auto transform = [&](const constant::Value* c) {
        auto create = [&](auto i) {
            return CreateScalar(source, c->Type(), decltype(i)(~i.value));
        };
        return Dispatch_ia_iu32(create, c);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::OpUnaryMinus(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    auto transform = [&](const constant::Value* c) {
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
                return CreateScalar(source, c->Type(), decltype(i)(v));
            } else {
                return CreateScalar(source, c->Type(), decltype(i)(-i.value));
            }
        };
        return Dispatch_fia_fi32_f16(create, c);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::OpNot(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c) {
        auto create = [&](auto i) { return CreateScalar(source, c->Type(), decltype(i)(!i)); };
        return Dispatch_bool(create, c);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::OpPlus(const type::Type* ty,
                                    utils::VectorRef<const constant::Value*> args,
                                    const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        return Dispatch_fia_fiu32_f16(AddFunc(source, c0->Type()), c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpMinus(const type::Type* ty,
                                     utils::VectorRef<const constant::Value*> args,
                                     const Source& source) {
    return Sub(source, ty, args[0], args[1]);
}

ConstEval::Result ConstEval::OpMultiply(const type::Type* ty,
                                        utils::VectorRef<const constant::Value*> args,
                                        const Source& source) {
    return Mul(source, ty, args[0], args[1]);
}

ConstEval::Result ConstEval::OpMultiplyMatVec(const type::Type* ty,
                                              utils::VectorRef<const constant::Value*> args,
                                              const Source& source) {
    auto* mat_ty = args[0]->Type()->As<type::Matrix>();
    auto* vec_ty = args[1]->Type()->As<type::Vector>();
    auto* elem_ty = vec_ty->type();

    auto dot = [&](const constant::Value* m, size_t row, const constant::Value* v) {
        ConstEval::Result result;
        switch (mat_ty->columns()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(source, elem_ty),  //
                                             m->Index(0)->Index(row),    //
                                             m->Index(1)->Index(row),    //
                                             v->Index(0),                //
                                             v->Index(1));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(source, elem_ty),  //
                                             m->Index(0)->Index(row),    //
                                             m->Index(1)->Index(row),    //
                                             m->Index(2)->Index(row),    //
                                             v->Index(0),                //
                                             v->Index(1), v->Index(2));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(source, elem_ty),  //
                                             m->Index(0)->Index(row),    //
                                             m->Index(1)->Index(row),    //
                                             m->Index(2)->Index(row),    //
                                             m->Index(3)->Index(row),    //
                                             v->Index(0),                //
                                             v->Index(1),                //
                                             v->Index(2),                //
                                             v->Index(3));
                break;
        }
        return result;
    };

    utils::Vector<const constant::Value*, 4> result;
    for (size_t i = 0; i < mat_ty->rows(); ++i) {
        auto r = dot(args[0], i, args[1]);  // matrix row i * vector
        if (!r) {
            return utils::Failure;
        }
        result.Push(r.Get());
    }
    return builder.create<constant::Composite>(ty, result);
}
ConstEval::Result ConstEval::OpMultiplyVecMat(const type::Type* ty,
                                              utils::VectorRef<const constant::Value*> args,
                                              const Source& source) {
    auto* vec_ty = args[0]->Type()->As<type::Vector>();
    auto* mat_ty = args[1]->Type()->As<type::Matrix>();
    auto* elem_ty = vec_ty->type();

    auto dot = [&](const constant::Value* v, const constant::Value* m, size_t col) {
        ConstEval::Result result;
        switch (mat_ty->rows()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(source, elem_ty),  //
                                             m->Index(col)->Index(0),    //
                                             m->Index(col)->Index(1),    //
                                             v->Index(0),                //
                                             v->Index(1));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(source, elem_ty),  //
                                             m->Index(col)->Index(0),    //
                                             m->Index(col)->Index(1),    //
                                             m->Index(col)->Index(2),
                                             v->Index(0),  //
                                             v->Index(1),  //
                                             v->Index(2));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(source, elem_ty),  //
                                             m->Index(col)->Index(0),    //
                                             m->Index(col)->Index(1),    //
                                             m->Index(col)->Index(2),    //
                                             m->Index(col)->Index(3),    //
                                             v->Index(0),                //
                                             v->Index(1),                //
                                             v->Index(2),                //
                                             v->Index(3));
        }
        return result;
    };

    utils::Vector<const constant::Value*, 4> result;
    for (size_t i = 0; i < mat_ty->columns(); ++i) {
        auto r = dot(args[0], args[1], i);  // vector * matrix col i
        if (!r) {
            return utils::Failure;
        }
        result.Push(r.Get());
    }
    return builder.create<constant::Composite>(ty, result);
}

ConstEval::Result ConstEval::OpMultiplyMatMat(const type::Type* ty,
                                              utils::VectorRef<const constant::Value*> args,
                                              const Source& source) {
    auto* mat1 = args[0];
    auto* mat2 = args[1];
    auto* mat1_ty = mat1->Type()->As<type::Matrix>();
    auto* mat2_ty = mat2->Type()->As<type::Matrix>();
    auto* elem_ty = mat1_ty->type();

    auto dot = [&](const constant::Value* m1, size_t row, const constant::Value* m2, size_t col) {
        auto m1e = [&](size_t r, size_t c) { return m1->Index(c)->Index(r); };
        auto m2e = [&](size_t r, size_t c) { return m2->Index(c)->Index(r); };

        ConstEval::Result result;
        switch (mat1_ty->columns()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(source, elem_ty),  //
                                             m1e(row, 0),                //
                                             m1e(row, 1),                //
                                             m2e(0, col),                //
                                             m2e(1, col));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(source, elem_ty),  //
                                             m1e(row, 0),                //
                                             m1e(row, 1),                //
                                             m1e(row, 2),                //
                                             m2e(0, col),                //
                                             m2e(1, col),                //
                                             m2e(2, col));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(source, elem_ty),  //
                                             m1e(row, 0),                //
                                             m1e(row, 1),                //
                                             m1e(row, 2),                //
                                             m1e(row, 3),                //
                                             m2e(0, col),                //
                                             m2e(1, col),                //
                                             m2e(2, col),                //
                                             m2e(3, col));
                break;
        }
        return result;
    };

    utils::Vector<const constant::Value*, 4> result_mat;
    for (size_t c = 0; c < mat2_ty->columns(); ++c) {
        utils::Vector<const constant::Value*, 4> col_vec;
        for (size_t r = 0; r < mat1_ty->rows(); ++r) {
            auto v = dot(mat1, r, mat2, c);  // mat1 row r * mat2 col c
            if (!v) {
                return utils::Failure;
            }
            col_vec.Push(v.Get());  // mat1 row r * mat2 col c
        }

        // Add column vector to matrix
        auto* col_vec_ty = ty->As<type::Matrix>()->ColumnType();
        result_mat.Push(builder.create<constant::Composite>(col_vec_ty, col_vec));
    }
    return builder.create<constant::Composite>(ty, result_mat);
}

ConstEval::Result ConstEval::OpDivide(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        return Dispatch_fia_fiu32_f16(DivFunc(source, c0->Type()), c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpModulo(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        return Dispatch_fia_fiu32_f16(ModFunc(source, c0->Type()), c0, c1);
    };

    return TransformBinaryElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpEqual(const type::Type* ty,
                                     utils::VectorRef<const constant::Value*> args,
                                     const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), i == j);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpNotEqual(const type::Type* ty,
                                        utils::VectorRef<const constant::Value*> args,
                                        const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), i != j);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpLessThan(const type::Type* ty,
                                        utils::VectorRef<const constant::Value*> args,
                                        const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), i < j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpGreaterThan(const type::Type* ty,
                                           utils::VectorRef<const constant::Value*> args,
                                           const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), i > j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpLessThanEqual(const type::Type* ty,
                                             utils::VectorRef<const constant::Value*> args,
                                             const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), i <= j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpGreaterThanEqual(const type::Type* ty,
                                                utils::VectorRef<const constant::Value*> args,
                                                const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), i >= j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpLogicalAnd(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    // Note: Due to short-circuiting, this function is only called if lhs is true, so we could
    // technically only return the value of the rhs.
    return CreateScalar(source, ty, args[0]->ValueAs<bool>() && args[1]->ValueAs<bool>());
}

ConstEval::Result ConstEval::OpLogicalOr(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    // Note: Due to short-circuiting, this function is only called if lhs is false, so we could
    // technically only return the value of the rhs.
    return CreateScalar(source, ty, args[1]->ValueAs<bool>());
}

ConstEval::Result ConstEval::OpAnd(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            using T = decltype(i);
            T result;
            if constexpr (std::is_same_v<T, bool>) {
                result = i && j;
            } else {  // integral
                result = i & j;
            }
            return CreateScalar(source, type::Type::DeepestElementOf(ty), result);
        };
        return Dispatch_ia_iu32_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpOr(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            using T = decltype(i);
            T result;
            if constexpr (std::is_same_v<T, bool>) {
                result = i || j;
            } else {  // integral
                result = i | j;
            }
            return CreateScalar(source, type::Type::DeepestElementOf(ty), result);
        };
        return Dispatch_ia_iu32_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpXor(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), decltype(i){i ^ j});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpShiftLeft(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto e1, auto e2) -> ConstEval::Result {
            using NumberT = decltype(e1);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr size_t bit_width = BitWidth<NumberT>;
            UT e1u = static_cast<UT>(e1);
            UT e2u = static_cast<UT>(e2);

            if constexpr (IsAbstract<NumberT>) {
                // The e2 + 1 most significant bits of e1 must have the same bit value, otherwise
                // sign change (overflow) would occur.
                // Check sign change only if e2 is less than bit width of e1. If e1 is larger
                // than bit width, we check for non-representable value below.
                if (e2u < bit_width) {
                    UT must_match_msb = e2u + 1;
                    UT mask = ~UT{0} << (bit_width - must_match_msb);
                    if ((e1u & mask) != 0 && (e1u & mask) != mask) {
                        AddError("shift left operation results in sign change", source);
                        if (!use_runtime_semantics_) {
                            return utils::Failure;
                        }
                    }
                } else {
                    // If shift value >= bit_width, then any non-zero value would overflow
                    if (e1 != 0) {
                        AddError(OverflowErrorMessage(e1, "<<", e2), source);
                        if (!use_runtime_semantics_) {
                            return utils::Failure;
                        }
                    }

                    // It's UB in C++ to shift by greater or equal to the bit width (even if the lhs
                    // is 0), so we make sure to avoid this by setting the shift value to 0.
                    e2u = 0;
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
                    if (use_runtime_semantics_) {
                        e2u = e2u % bit_width;
                    } else {
                        return utils::Failure;
                    }
                }

                if constexpr (std::is_signed_v<T>) {
                    // If T is a signed integer type, and the e2+1 most significant bits of e1 do
                    // not have the same bit value, then error.
                    size_t must_match_msb = e2u + 1;
                    UT mask = ~UT{0} << (bit_width - must_match_msb);
                    if ((e1u & mask) != 0 && (e1u & mask) != mask) {
                        AddError("shift left operation results in sign change", source);
                        if (!use_runtime_semantics_) {
                            return utils::Failure;
                        }
                    }
                } else {
                    // If T is an unsigned integer type, and any of the e2 most significant bits of
                    // e1 are 1, then error.
                    if (e2u > 0) {
                        size_t must_be_zero_msb = e2u;
                        UT mask = ~UT{0} << (bit_width - must_be_zero_msb);
                        if ((e1u & mask) != 0) {
                            AddError(OverflowErrorMessage(e1, "<<", e2), source);
                            if (!use_runtime_semantics_) {
                                return utils::Failure;
                            }
                        }
                    }
                }
            }

            // Avoid UB by left shifting as unsigned value
            auto result = static_cast<T>(static_cast<UT>(e1) << e2u);
            return CreateScalar(source, type::Type::DeepestElementOf(ty), NumberT{result});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    if (TINT_UNLIKELY(!type::Type::DeepestElementOf(args[1]->Type())->Is<type::U32>())) {
        TINT_ICE(Resolver, builder.Diagnostics())
            << "Element type of rhs of ShiftLeft must be a u32";
        return utils::Failure;
    }

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::OpShiftRight(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto e1, auto e2) -> ConstEval::Result {
            using NumberT = decltype(e1);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            const size_t bit_width = BitWidth<NumberT>;
            UT e1u = static_cast<UT>(e1);
            UT e2u = static_cast<UT>(e2);

            auto signed_shift_right = [&] {
                // In C++, right shift of a signed negative number is implementation-defined.
                // Although most implementations sign-extend, we do it manually to ensure it works
                // correctly on all implementations.
                const UT msb = UT{1} << (bit_width - 1);
                UT sign_ext = 0;
                if (e1u & msb) {
                    // Set e2 + 1 bits to 1
                    UT num_shift_bits_mask = ((UT{1} << e2u) - UT{1});
                    sign_ext = (num_shift_bits_mask << (bit_width - e2u - UT{1})) | msb;
                }
                return static_cast<T>((e1u >> e2u) | sign_ext);
            };

            T result = 0;
            if constexpr (IsAbstract<NumberT>) {
                if (static_cast<size_t>(e2) >= bit_width) {
                    result = T{0};
                } else {
                    result = signed_shift_right();
                }
            } else {
                if (static_cast<size_t>(e2) >= bit_width) {
                    // At shader/pipeline-creation time, it is an error to shift by the bit width of
                    // the lhs or greater. NOTE: At runtime, we shift by e2 % (bit width of e1).
                    AddError(
                        "shift right value must be less than the bit width of the lhs, which is " +
                            std::to_string(bit_width),
                        source);
                    if (use_runtime_semantics_) {
                        e2u = e2u % bit_width;
                    } else {
                        return utils::Failure;
                    }
                }

                if constexpr (std::is_signed_v<T>) {
                    result = signed_shift_right();
                } else {
                    result = e1 >> e2u;
                }
            }
            return CreateScalar(source, type::Type::DeepestElementOf(ty), NumberT{result});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    if (TINT_UNLIKELY(!type::Type::DeepestElementOf(args[1]->Type())->Is<type::U32>())) {
        TINT_ICE(Resolver, builder.Diagnostics())
            << "Element type of rhs of ShiftLeft must be a u32";
        return utils::Failure;
    }

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::abs(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            NumberT result;
            if constexpr (IsUnsignedIntegral<NumberT>) {
                result = e;
            } else if constexpr (IsSignedIntegral<NumberT>) {
                if (e == NumberT::Lowest()) {
                    result = e;
                } else {
                    result = NumberT{std::abs(e)};
                }
            } else {
                result = NumberT{std::abs(e)};
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fia_fiu32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::acos(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            if (i < NumberT(-1.0) || i > NumberT(1.0)) {
                AddError("acos must be called with a value in the range [-1 .. 1] (inclusive)",
                         source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::acos(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::acosh(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            if (i < NumberT(1.0)) {
                AddError("acosh must be called with a value >= 1.0", source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::acosh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::all(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    return CreateScalar(source, ty, !args[0]->AnyZero());
}

ConstEval::Result ConstEval::any(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    return CreateScalar(source, ty, !args[0]->AllZero());
}

ConstEval::Result ConstEval::asin(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            if (i < NumberT(-1.0) || i > NumberT(1.0)) {
                AddError("asin must be called with a value in the range [-1 .. 1] (inclusive)",
                         source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::asin(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::asinh(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::asinh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::atan(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::atan(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::atanh(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            if (i <= NumberT(-1.0) || i >= NumberT(1.0)) {
                AddError("atanh must be called with a value in the range (-1 .. 1) (exclusive)",
                         source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::atanh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::atan2(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto i, auto j) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::atan2(i.value, j.value)));
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::ceil(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            return CreateScalar(source, c0->Type(), decltype(e)(std::ceil(e)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::clamp(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1,
                         const constant::Value* c2) {
        return Dispatch_fia_fiu32_f16(ClampFunc(source, c0->Type()), c0, c1, c2);
    };
    return TransformElements(builder, ty, transform, args[0], args[1], args[2]);
}

ConstEval::Result ConstEval::cos(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::cos(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::cosh(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::cosh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::countLeadingZeros(const type::Type* ty,
                                               utils::VectorRef<const constant::Value*> args,
                                               const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            auto count = CountLeadingBits(T{e}, T{0});
            return CreateScalar(source, c0->Type(), NumberT(count));
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::countOneBits(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr UT kRightMost = UT{1};

            auto count = UT{0};
            for (auto v = static_cast<UT>(e); v != UT{0}; v >>= 1) {
                if ((v & kRightMost) == 1) {
                    ++count;
                }
            }

            return CreateScalar(source, c0->Type(), NumberT(count));
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::countTrailingZeros(const type::Type* ty,
                                                utils::VectorRef<const constant::Value*> args,
                                                const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            auto count = CountTrailingBits(T{e}, T{0});
            return CreateScalar(source, c0->Type(), NumberT(count));
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::cross(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto* u = args[0];
    auto* v = args[1];
    auto* elem_ty = u->Type()->As<type::Vector>()->type();

    // cross product of a v3 is the determinant of the 3x3 matrix:
    //
    // |i   j   k |
    // |u0  u1  u2|
    // |v0  v1  v2|
    //
    // |u1 u2|i  - |u0 u2|j + |u0 u1|k
    // |v1 v2|     |v0 v2|    |v0 v1|
    //
    // |u1 u2|i  + |v0 v2|j + |u0 u1|k
    // |v1 v2|     |u0 u2|    |v0 v1|

    auto* u0 = u->Index(0);
    auto* u1 = u->Index(1);
    auto* u2 = u->Index(2);
    auto* v0 = v->Index(0);
    auto* v1 = v->Index(1);
    auto* v2 = v->Index(2);

    auto x = Dispatch_fa_f32_f16(Det2Func(source, elem_ty), u1, u2, v1, v2);
    if (!x) {
        return utils::Failure;
    }
    auto y = Dispatch_fa_f32_f16(Det2Func(source, elem_ty), v0, v2, u0, u2);
    if (!y) {
        return utils::Failure;
    }
    auto z = Dispatch_fa_f32_f16(Det2Func(source, elem_ty), u0, u1, v0, v1);
    if (!z) {
        return utils::Failure;
    }

    return builder.create<constant::Composite>(
        ty, utils::Vector<const constant::Value*, 3>{x.Get(), y.Get(), z.Get()});
}

ConstEval::Result ConstEval::degrees(const type::Type* ty,
                                     utils::VectorRef<const constant::Value*> args,
                                     const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) -> ConstEval::Result {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;

            auto pi = kPi<T>;
            auto scale = Div(source, NumberT(180), NumberT(pi));
            if (!scale) {
                AddNote("when calculating degrees", source);
                return utils::Failure;
            }
            auto result = Mul(source, e, scale.Get());
            if (!result) {
                AddNote("when calculating degrees", source);
                return utils::Failure;
            }
            return CreateScalar(source, c0->Type(), result.Get());
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::determinant(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    auto calculate = [&]() -> ConstEval::Result {
        auto* m = args[0];
        auto* mat_ty = m->Type()->As<type::Matrix>();
        auto me = [&](size_t r, size_t c) { return m->Index(c)->Index(r); };
        switch (mat_ty->rows()) {
            case 2:
                return Dispatch_fa_f32_f16(Det2Func(source, ty),  //
                                           me(0, 0), me(1, 0),    //
                                           me(0, 1), me(1, 1));

            case 3:
                return Dispatch_fa_f32_f16(Det3Func(source, ty),          //
                                           me(0, 0), me(1, 0), me(2, 0),  //
                                           me(0, 1), me(1, 1), me(2, 1),  //
                                           me(0, 2), me(1, 2), me(2, 2));

            case 4:
                return Dispatch_fa_f32_f16(Det4Func(source, ty),                    //
                                           me(0, 0), me(1, 0), me(2, 0), me(3, 0),  //
                                           me(0, 1), me(1, 1), me(2, 1), me(3, 1),  //
                                           me(0, 2), me(1, 2), me(2, 2), me(3, 2),  //
                                           me(0, 3), me(1, 3), me(2, 3), me(3, 3));
        }
        TINT_ICE(Resolver, builder.Diagnostics()) << "Unexpected number of matrix rows";
        return utils::Failure;
    };
    auto r = calculate();
    if (!r) {
        AddNote("when calculating determinant", source);
    }
    return r;
}

ConstEval::Result ConstEval::distance(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source& source) {
    auto err = [&]() -> ConstEval::Result {
        AddNote("when calculating distance", source);
        return utils::Failure;
    };

    auto minus = OpMinus(args[0]->Type(), args, source);
    if (!minus) {
        return err();
    }

    auto len = Length(source, ty, minus.Get());
    if (!len) {
        return err();
    }
    return len;
}

ConstEval::Result ConstEval::dot(const type::Type*,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto r = Dot(source, args[0], args[1]);
    if (!r) {
        AddNote("when calculating dot", source);
    }
    return r;
}

ConstEval::Result ConstEval::exp(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e0) -> ConstEval::Result {
            using NumberT = decltype(e0);
            auto val = NumberT(std::exp(e0));
            if (!std::isfinite(val.value)) {
                AddError(OverflowExpErrorMessage("e", e0), source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), val);
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::exp2(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e0) -> ConstEval::Result {
            using NumberT = decltype(e0);
            auto val = NumberT(std::exp2(e0));
            if (!std::isfinite(val.value)) {
                AddError(OverflowExpErrorMessage("2", e0), source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), val);
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::extractBits(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto in_e) -> ConstEval::Result {
            using NumberT = decltype(in_e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            using NumberUT = Number<UT>;

            // Read args that are always scalar
            NumberUT in_offset = args[1]->ValueAs<NumberUT>();
            NumberUT in_count = args[2]->ValueAs<NumberUT>();

            // Cast all to unsigned
            UT e = static_cast<UT>(in_e);
            UT o = static_cast<UT>(in_offset);
            UT c = static_cast<UT>(in_count);

            constexpr UT w = sizeof(UT) * 8;
            if (o > w || c > w || (o + c) > w) {
                AddError("'offset + 'count' must be less than or equal to the bit width of 'e'",
                         source);
                if (use_runtime_semantics_) {
                    o = std::min(o, w);
                    c = std::min(c, w - o);
                } else {
                    return utils::Failure;
                }
            }

            NumberT result;
            if (c == UT{0}) {
                // The result is 0 if c is 0
                result = NumberT{0};
            } else if (c == w) {
                // The result is e if c is w
                result = NumberT{e};
            } else {
                // Otherwise, bits 0..c - 1 of the result are copied from bits o..o + c - 1 of e.
                UT src_mask = ((UT{1} << c) - UT{1}) << o;
                UT r = (e & src_mask) >> o;
                if constexpr (IsSignedIntegral<NumberT>) {
                    // Other bits of the result are the same as bit c - 1 of the result.
                    // Only need to set other bits if bit at c - 1 of result is 1
                    if ((r & (UT{1} << (c - UT{1}))) != UT{0}) {
                        UT dst_mask = src_mask >> o;
                        r |= (~UT{0} & ~dst_mask);
                    }
                }

                result = NumberT{r};
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::faceForward(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    // Returns e1 if dot(e2, e3) is negative, and -e1 otherwise.
    auto* e1 = args[0];
    auto* e2 = args[1];
    auto* e3 = args[2];
    auto r = Dot(source, e2, e3);
    if (!r) {
        AddNote("when calculating faceForward", source);
        return utils::Failure;
    }
    auto is_negative = [](auto v) { return v < 0; };
    if (Dispatch_fa_f32_f16(is_negative, r.Get())) {
        return e1;
    }
    return OpUnaryMinus(ty, utils::Vector{e1}, source);
}

ConstEval::Result ConstEval::firstLeadingBit(const type::Type* ty,
                                             utils::VectorRef<const constant::Value*> args,
                                             const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr UT kNumBits = sizeof(UT) * 8;

            NumberT result;
            if constexpr (IsUnsignedIntegral<T>) {
                if (e == T{0}) {
                    // T(-1) if e is zero.
                    result = NumberT(static_cast<T>(-1));
                } else {
                    // Otherwise the position of the most significant 1 bit in e.
                    static_assert(std::is_same_v<T, UT>);
                    UT count = CountLeadingBits(UT{e}, UT{0});
                    UT pos = kNumBits - count - 1;
                    result = NumberT(pos);
                }
            } else {
                if (e == T{0} || e == T{-1}) {
                    // -1 if e is 0 or -1.
                    result = NumberT(-1);
                } else {
                    // Otherwise the position of the most significant bit in e that is different
                    // from e's sign bit.
                    UT eu = static_cast<UT>(e);
                    UT sign_bit = eu >> (kNumBits - 1);
                    UT count = CountLeadingBits(eu, sign_bit);
                    UT pos = kNumBits - count - 1;
                    result = NumberT(pos);
                }
            }

            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::firstTrailingBit(const type::Type* ty,
                                              utils::VectorRef<const constant::Value*> args,
                                              const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;

            NumberT result;
            if (e == T{0}) {
                // T(-1) if e is zero.
                result = NumberT(static_cast<T>(-1));
            } else {
                // Otherwise the position of the least significant 1 bit in e.
                UT pos = CountTrailingBits(T{e}, T{0});
                result = NumberT(pos);
            }

            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::floor(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            return CreateScalar(source, c0->Type(), decltype(e)(std::floor(e)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::fma(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c1, const constant::Value* c2,
                         const constant::Value* c3) {
        auto create = [&](auto e1, auto e2, auto e3) -> ConstEval::Result {
            auto err_msg = [&] {
                AddNote("when calculating fma", source);
                return utils::Failure;
            };

            auto mul = Mul(source, e1, e2);
            if (!mul) {
                return err_msg();
            }

            auto val = Add(source, mul.Get(), e3);
            if (!val) {
                return err_msg();
            }
            return CreateScalar(source, c1->Type(), val.Get());
        };
        return Dispatch_fa_f32_f16(create, c1, c2, c3);
    };
    return TransformElements(builder, ty, transform, args[0], args[1], args[2]);
}

ConstEval::Result ConstEval::fract(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c1) {
        auto create = [&](auto e) -> ConstEval::Result {
            using NumberT = decltype(e);
            auto r = e - std::floor(e);
            return CreateScalar(source, c1->Type(), NumberT{r});
        };
        return Dispatch_fa_f32_f16(create, c1);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::frexp(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto* arg = args[0];

    struct FractExp {
        ConstEval::Result fract;
        ConstEval::Result exp;
    };

    auto scalar = [&](const constant::Value* s) {
        int exp = 0;
        double fract = std::frexp(s->ValueAs<AFloat>(), &exp);
        return Switch(
            s->Type(),
            [&](const type::F32*) {
                return FractExp{
                    CreateScalar(source, builder.create<type::F32>(), f32(fract)),
                    CreateScalar(source, builder.create<type::I32>(), i32(exp)),
                };
            },
            [&](const type::F16*) {
                return FractExp{
                    CreateScalar(source, builder.create<type::F16>(), f16(fract)),
                    CreateScalar(source, builder.create<type::I32>(), i32(exp)),
                };
            },
            [&](const type::AbstractFloat*) {
                return FractExp{
                    CreateScalar(source, builder.create<type::AbstractFloat>(), AFloat(fract)),
                    CreateScalar(source, builder.create<type::AbstractInt>(), AInt(exp)),
                };
            },
            [&](Default) {
                TINT_ICE(Resolver, builder.Diagnostics())
                    << "unhandled element type for frexp() const-eval: "
                    << s->Type()->FriendlyName();
                return FractExp{utils::Failure, utils::Failure};
            });
    };

    if (auto* vec = arg->Type()->As<type::Vector>()) {
        utils::Vector<const constant::Value*, 4> fract_els;
        utils::Vector<const constant::Value*, 4> exp_els;
        for (uint32_t i = 0; i < vec->Width(); i++) {
            auto fe = scalar(arg->Index(i));
            if (!fe.fract || !fe.exp) {
                return utils::Failure;
            }
            fract_els.Push(fe.fract.Get());
            exp_els.Push(fe.exp.Get());
        }
        auto fract_ty = builder.create<type::Vector>(fract_els[0]->Type(), vec->Width());
        auto exp_ty = builder.create<type::Vector>(exp_els[0]->Type(), vec->Width());
        return builder.create<constant::Composite>(
            ty, utils::Vector<const constant::Value*, 2>{
                    builder.create<constant::Composite>(fract_ty, std::move(fract_els)),
                    builder.create<constant::Composite>(exp_ty, std::move(exp_els)),
                });
    } else {
        auto fe = scalar(arg);
        if (!fe.fract || !fe.exp) {
            return utils::Failure;
        }
        return builder.create<constant::Composite>(ty, utils::Vector<const constant::Value*, 2>{
                                                           fe.fract.Get(),
                                                           fe.exp.Get(),
                                                       });
    }
}

ConstEval::Result ConstEval::insertBits(const type::Type* ty,
                                        utils::VectorRef<const constant::Value*> args,
                                        const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto in_e, auto in_newbits) -> ConstEval::Result {
            using NumberT = decltype(in_e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            using NumberUT = Number<UT>;

            // Read args that are always scalar
            NumberUT in_offset = args[2]->ValueAs<NumberUT>();
            NumberUT in_count = args[3]->ValueAs<NumberUT>();

            // Cast all to unsigned
            UT e = static_cast<UT>(in_e);
            UT newbits = static_cast<UT>(in_newbits);
            UT o = static_cast<UT>(in_offset);
            UT c = static_cast<UT>(in_count);

            constexpr UT w = sizeof(UT) * 8;
            if (o > w || c > w || (o + c) > w) {
                AddError("'offset + 'count' must be less than or equal to the bit width of 'e'",
                         source);
                if (use_runtime_semantics_) {
                    o = std::min(o, w);
                    c = std::min(c, w - o);
                } else {
                    return utils::Failure;
                }
            }

            NumberT result;
            if (c == UT{0}) {
                // The result is e if c is 0
                result = NumberT{e};
            } else if (c == w) {
                // The result is newbits if c is w
                result = NumberT{newbits};
            } else {
                // Otherwise, bits o..o + c - 1 of the result are copied from bits 0..c - 1 of
                // newbits. Other bits of the result are copied from e.
                UT from = newbits << o;
                UT mask = ((UT{1} << c) - UT{1}) << UT{o};
                auto r = e;          // Start with 'e' as the result
                r &= ~mask;          // Zero the bits in 'e' we're overwriting
                r |= (from & mask);  // Overwrite from 'newbits' (shifted into position)
                result = NumberT{r};
            }

            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_iu32(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::inverseSqrt(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) -> ConstEval::Result {
            using NumberT = decltype(e);

            if (e <= NumberT(0)) {
                AddError("inverseSqrt must be called with a value > 0", source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }

            auto err = [&] {
                AddNote("when calculating inverseSqrt", source);
                return utils::Failure;
            };

            auto s = Sqrt(source, e);
            if (!s) {
                return err();
            }
            auto div = Div(source, NumberT(1), s.Get());
            if (!div) {
                return err();
            }

            return CreateScalar(source, c0->Type(), div.Get());
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::ldexp(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c1, size_t index) {
        auto create = [&](auto e1) -> ConstEval::Result {
            using E1Type = decltype(e1);
            // If e1 is AFloat, then e2 is AInt, otherwise it's i32
            using E2Type = std::conditional_t<std::is_same_v<E1Type, AFloat>, AInt, i32>;

            E2Type e2;
            auto* c2 = args[1];
            if (c2->Type()->Is<type::Vector>()) {
                e2 = c2->Index(index)->ValueAs<E2Type>();
            } else {
                e2 = c2->ValueAs<E2Type>();
            }

            E2Type bias;
            if constexpr (std::is_same_v<E1Type, f16>) {
                bias = 15;
            } else if constexpr (std::is_same_v<E1Type, f32>) {
                bias = 127;
            } else {
                bias = 1023;
            }

            if (e2 > bias + 1) {
                AddError("e2 must be less than or equal to " + std::to_string(bias + 1), source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c1->Type());
                } else {
                    return utils::Failure;
                }
            }

            auto target_ty = type::Type::DeepestElementOf(ty);

            auto r = std::ldexp(e1, static_cast<int>(e2));
            return CreateScalar(source, target_ty, E1Type{r});
        };
        return Dispatch_fa_f32_f16(create, c1);
    };

    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::length(const type::Type* ty,
                                    utils::VectorRef<const constant::Value*> args,
                                    const Source& source) {
    auto r = Length(source, ty, args[0]);
    if (!r) {
        AddNote("when calculating length", source);
    }
    return r;
}

ConstEval::Result ConstEval::log(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto v) -> ConstEval::Result {
            using NumberT = decltype(v);
            if (v <= NumberT(0)) {
                AddError("log must be called with a value > 0", source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::log(v)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::log2(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto v) -> ConstEval::Result {
            using NumberT = decltype(v);
            if (v <= NumberT(0)) {
                AddError("log2 must be called with a value > 0", source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::log2(v)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::max(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto e0, auto e1) {
            return CreateScalar(source, c0->Type(), decltype(e0)(std::max(e0, e1)));
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::min(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto e0, auto e1) {
            return CreateScalar(source, c0->Type(), decltype(e0)(std::min(e0, e1)));
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::mix(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1, size_t index) {
        auto create = [&](auto e1, auto e2) -> ConstEval::Result {
            using NumberT = decltype(e1);
            // e3 is either a vector or a scalar
            NumberT e3;
            auto* c2 = args[2];
            if (c2->Type()->Is<type::Vector>()) {
                e3 = c2->Index(index)->ValueAs<NumberT>();
            } else {
                e3 = c2->ValueAs<NumberT>();
            }
            // Implement as `e1 * (1 - e3) + e2 * e3)` instead of as `e1 + e3 * (e2 - e1)` to avoid
            // float precision loss when e1 and e2 significantly differ in magnitude.
            auto one_sub_e3 = Sub(source, NumberT{1}, e3);
            if (!one_sub_e3) {
                return utils::Failure;
            }
            auto e1_mul_one_sub_e3 = Mul(source, e1, one_sub_e3.Get());
            if (!e1_mul_one_sub_e3) {
                return utils::Failure;
            }
            auto e2_mul_e3 = Mul(source, e2, e3);
            if (!e2_mul_e3) {
                return utils::Failure;
            }
            auto r = Add(source, e1_mul_one_sub_e3.Get(), e2_mul_e3.Get());
            if (!r) {
                return utils::Failure;
            }
            return CreateScalar(source, c0->Type(), r.Get());
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    auto r = TransformElements(builder, ty, transform, args[0], args[1]);
    if (!r) {
        AddNote("when calculating mix", source);
    }
    return r;
}

ConstEval::Result ConstEval::modf(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform_fract = [&](const constant::Value* c) {
        auto create = [&](auto e) {
            return CreateScalar(source, c->Type(), decltype(e)(e.value - std::trunc(e.value)));
        };
        return Dispatch_fa_f32_f16(create, c);
    };
    auto transform_whole = [&](const constant::Value* c) {
        auto create = [&](auto e) {
            return CreateScalar(source, c->Type(), decltype(e)(std::trunc(e.value)));
        };
        return Dispatch_fa_f32_f16(create, c);
    };

    utils::Vector<const constant::Value*, 2> fields;

    if (auto fract = TransformElements(builder, args[0]->Type(), transform_fract, args[0])) {
        fields.Push(fract.Get());
    } else {
        return utils::Failure;
    }

    if (auto whole = TransformElements(builder, args[0]->Type(), transform_whole, args[0])) {
        fields.Push(whole.Get());
    } else {
        return utils::Failure;
    }

    return builder.create<constant::Composite>(ty, std::move(fields));
}

ConstEval::Result ConstEval::normalize(const type::Type* ty,
                                       utils::VectorRef<const constant::Value*> args,
                                       const Source& source) {
    auto* len_ty = type::Type::DeepestElementOf(ty);
    auto len = Length(source, len_ty, args[0]);
    if (!len) {
        AddNote("when calculating normalize", source);
        return utils::Failure;
    }
    auto* v = len.Get();
    if (v->AllZero()) {
        AddError("zero length vector can not be normalized", source);
        if (use_runtime_semantics_) {
            return ZeroValue(ty);
        } else {
            return utils::Failure;
        }
    }
    return OpDivide(ty, utils::Vector{args[0], v}, source);
}

ConstEval::Result ConstEval::pack2x16float(const type::Type* ty,
                                           utils::VectorRef<const constant::Value*> args,
                                           const Source& source) {
    auto convert = [&](f32 val) -> utils::Result<uint32_t> {
        auto conv = CheckedConvert<f16>(val);
        if (!conv) {
            AddError(OverflowErrorMessage(val, "f16"), source);
            if (use_runtime_semantics_) {
                return 0;
            } else {
                return utils::Failure;
            }
        }
        uint16_t v = conv.Get().BitsRepresentation();
        return utils::Result<uint32_t>{v};
    };

    auto* e = args[0];
    auto e0 = convert(e->Index(0)->ValueAs<f32>());
    if (!e0) {
        return utils::Failure;
    }

    auto e1 = convert(e->Index(1)->ValueAs<f32>());
    if (!e1) {
        return utils::Failure;
    }

    u32 ret = u32((e0.Get() & 0x0000'ffff) | (e1.Get() << 16));
    return CreateScalar(source, ty, ret);
}

ConstEval::Result ConstEval::pack2x16snorm(const type::Type* ty,
                                           utils::VectorRef<const constant::Value*> args,
                                           const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(-1.0f), f32(1.0f)).Get();
        return u32(utils::Bitcast<uint16_t>(
            static_cast<int16_t>(std::floor(0.5f + (32767.0f * clamped)))));
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());

    u32 ret = u32((e0 & 0x0000'ffff) | (e1 << 16));
    return CreateScalar(source, ty, ret);
}

ConstEval::Result ConstEval::pack2x16unorm(const type::Type* ty,
                                           utils::VectorRef<const constant::Value*> args,
                                           const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(0.0f), f32(1.0f)).Get();
        return u32{std::floor(0.5f + (65535.0f * clamped))};
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());

    u32 ret = u32((e0 & 0x0000'ffff) | (e1 << 16));
    return CreateScalar(source, ty, ret);
}

ConstEval::Result ConstEval::pack4x8snorm(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(-1.0f), f32(1.0f)).Get();
        return u32(
            utils::Bitcast<uint8_t>(static_cast<int8_t>(std::floor(0.5f + (127.0f * clamped)))));
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());
    auto e2 = calc(e->Index(2)->ValueAs<f32>());
    auto e3 = calc(e->Index(3)->ValueAs<f32>());

    uint32_t mask = 0x0000'00ff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

ConstEval::Result ConstEval::pack4x8unorm(const type::Type* ty,
                                          utils::VectorRef<const constant::Value*> args,
                                          const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(0.0f), f32(1.0f)).Get();
        return u32{std::floor(0.5f + (255.0f * clamped))};
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());
    auto e2 = calc(e->Index(2)->ValueAs<f32>());
    auto e3 = calc(e->Index(3)->ValueAs<f32>());

    uint32_t mask = 0x0000'00ff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

ConstEval::Result ConstEval::pow(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto e1, auto e2) -> ConstEval::Result {
            auto r = CheckedPow(e1, e2);
            if (!r) {
                AddError(OverflowErrorMessage(e1, "^", e2), source);
                if (use_runtime_semantics_) {
                    return ZeroValue(c0->Type());
                } else {
                    return utils::Failure;
                }
            }
            return CreateScalar(source, c0->Type(), *r);
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::radians(const type::Type* ty,
                                     utils::VectorRef<const constant::Value*> args,
                                     const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) -> ConstEval::Result {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;

            auto pi = kPi<T>;
            auto scale = Div(source, NumberT(pi), NumberT(180));
            if (!scale) {
                AddNote("when calculating radians", source);
                return utils::Failure;
            }
            auto result = Mul(source, e, scale.Get());
            if (!result) {
                AddNote("when calculating radians", source);
                return utils::Failure;
            }
            return CreateScalar(source, c0->Type(), result.Get());
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::reflect(const type::Type* ty,
                                     utils::VectorRef<const constant::Value*> args,
                                     const Source& source) {
    auto calculate = [&]() -> ConstEval::Result {
        // For the incident vector e1 and surface orientation e2, returns the reflection direction
        // e1 - 2 * dot(e2, e1) * e2.
        auto* e1 = args[0];
        auto* e2 = args[1];
        auto* vec_ty = ty->As<type::Vector>();
        auto* el_ty = vec_ty->type();

        // dot(e2, e1)
        auto dot_e2_e1 = Dot(source, e2, e1);
        if (!dot_e2_e1) {
            return utils::Failure;
        }

        // 2 * dot(e2, e1)
        auto mul2 = [&](auto v) -> ConstEval::Result {
            using NumberT = decltype(v);
            return CreateScalar(source, el_ty, NumberT{NumberT{2} * v});
        };
        auto dot_e2_e1_2 = Dispatch_fa_f32_f16(mul2, dot_e2_e1.Get());
        if (!dot_e2_e1_2) {
            return utils::Failure;
        }

        // 2 * dot(e2, e1) * e2
        auto dot_e2_e1_2_e2 = Mul(source, ty, dot_e2_e1_2.Get(), e2);
        if (!dot_e2_e1_2_e2) {
            return utils::Failure;
        }

        // e1 - 2 * dot(e2, e1) * e2
        return Sub(source, ty, e1, dot_e2_e1_2_e2.Get());
    };
    auto r = calculate();
    if (!r) {
        AddNote("when calculating reflect", source);
    }
    return r;
}

ConstEval::Result ConstEval::refract(const type::Type* ty,
                                     utils::VectorRef<const constant::Value*> args,
                                     const Source& source) {
    auto* vec_ty = ty->As<type::Vector>();
    auto* el_ty = vec_ty->type();

    auto compute_k = [&](auto e3, auto dot_e2_e1) -> ConstEval::Result {
        using NumberT = decltype(e3);
        // let k = 1.0 - e3 * e3 * (1.0 - dot(e2, e1) * dot(e2, e1))
        auto e3_squared = Mul(source, e3, e3);
        if (!e3_squared) {
            return utils::Failure;
        }
        auto dot_e2_e1_squared = Mul(source, dot_e2_e1, dot_e2_e1);
        if (!dot_e2_e1_squared) {
            return utils::Failure;
        }
        auto r = Sub(source, NumberT(1), dot_e2_e1_squared.Get());
        if (!r) {
            return utils::Failure;
        }
        r = Mul(source, e3_squared.Get(), r.Get());
        if (!r) {
            return utils::Failure;
        }
        r = Sub(source, NumberT(1), r.Get());
        if (!r) {
            return utils::Failure;
        }
        return CreateScalar(source, el_ty, r.Get());
    };

    auto compute_e2_scale = [&](auto e3, auto dot_e2_e1, auto k) -> ConstEval::Result {
        // e3 * dot(e2, e1) + sqrt(k)
        auto sqrt_k = Sqrt(source, k);
        if (!sqrt_k) {
            return utils::Failure;
        }
        auto r = Mul(source, e3, dot_e2_e1);
        if (!r) {
            return utils::Failure;
        }
        r = Add(source, r.Get(), sqrt_k.Get());
        if (!r) {
            return utils::Failure;
        }
        return CreateScalar(source, el_ty, r.Get());
    };

    auto calculate = [&]() -> ConstEval::Result {
        auto* e1 = args[0];
        auto* e2 = args[1];
        auto* e3 = args[2];

        // For the incident vector e1 and surface normal e2, and the ratio of indices of refraction
        // e3, let k = 1.0 - e3 * e3 * (1.0 - dot(e2, e1) * dot(e2, e1)). If k < 0.0, returns the
        // refraction vector 0.0, otherwise return the refraction vector e3 * e1 - (e3 * dot(e2, e1)
        // + sqrt(k)) * e2.

        // dot(e2, e1)
        auto dot_e2_e1 = Dot(source, e2, e1);
        if (!dot_e2_e1) {
            return utils::Failure;
        }

        // let k = 1.0 - e3 * e3 * (1.0 - dot(e2, e1) * dot(e2, e1))
        auto k = Dispatch_fa_f32_f16(compute_k, e3, dot_e2_e1.Get());
        if (!k) {
            return utils::Failure;
        }

        // If k < 0.0, returns the refraction vector 0.0
        if (k.Get()->ValueAs<AFloat>() < 0) {
            return ZeroValue(ty);
        }

        // Otherwise return the refraction vector e3 * e1 - (e3 * dot(e2, e1) + sqrt(k)) * e2
        auto e1_scaled = Mul(source, ty, e3, e1);
        if (!e1_scaled) {
            return utils::Failure;
        }
        auto e2_scale = Dispatch_fa_f32_f16(compute_e2_scale, e3, dot_e2_e1.Get(), k.Get());
        if (!e2_scale) {
            return utils::Failure;
        }
        auto e2_scaled = Mul(source, ty, e2_scale.Get(), e2);
        if (!e2_scaled) {
            return utils::Failure;
        }
        return Sub(source, ty, e1_scaled.Get(), e2_scaled.Get());
    };
    auto r = calculate();
    if (!r) {
        AddNote("when calculating refract", source);
    }
    return r;
}

ConstEval::Result ConstEval::reverseBits(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto in_e) -> ConstEval::Result {
            using NumberT = decltype(in_e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr UT kNumBits = sizeof(UT) * 8;

            UT e = static_cast<UT>(in_e);
            UT r = UT{0};
            for (size_t s = 0; s < kNumBits; ++s) {
                // Write source 's' bit to destination 'd' bit if 1
                if (e & (UT{1} << s)) {
                    size_t d = kNumBits - s - 1;
                    r |= (UT{1} << d);
                }
            }

            return CreateScalar(source, c0->Type(), NumberT{r});
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::round(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;

            auto integral = NumberT(0);
            auto fract = std::abs(std::modf(e.value, &(integral.value)));
            // When e lies halfway between integers k and k + 1, the result is k when k is even,
            // and k + 1 when k is odd.
            NumberT result = NumberT(0.0);
            if (fract == NumberT(0.5)) {
                // If the integral value is negative, then we need to subtract one in order to move
                // to the correct `k`. The half way check is `k` and `k + 1` which in the positive
                // case is `x` and `x + 1` but in the negative case is `x - 1` and `x`.
                T integral_val = integral.value;
                if (std::signbit(integral_val)) {
                    integral_val = std::abs(integral_val - 1);
                }
                if (uint64_t(integral_val) % 2 == 0) {
                    result = NumberT(std::floor(e.value));
                } else {
                    result = NumberT(std::ceil(e.value));
                }
            } else {
                result = NumberT(std::round(e.value));
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::saturate(const type::Type* ty,
                                      utils::VectorRef<const constant::Value*> args,
                                      const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            return CreateScalar(source, c0->Type(),
                                NumberT(std::min(std::max(e, NumberT(0.0)), NumberT(1.0))));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::select_bool(const type::Type* ty,
                                         utils::VectorRef<const constant::Value*> args,
                                         const Source& source) {
    auto cond = args[2]->ValueAs<bool>();
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto f, auto t) -> ConstEval::Result {
            return CreateScalar(source, type::Type::DeepestElementOf(ty), cond ? t : f);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::select_boolvec(const type::Type* ty,
                                            utils::VectorRef<const constant::Value*> args,
                                            const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1, size_t index) {
        auto create = [&](auto f, auto t) -> ConstEval::Result {
            // Get corresponding bool value at the current vector value index
            auto cond = args[2]->Index(index)->ValueAs<bool>();
            return CreateScalar(source, type::Type::DeepestElementOf(ty), cond ? t : f);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::sign(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto e) -> ConstEval::Result {
            using NumberT = decltype(e);
            NumberT result;
            NumberT zero{0.0};
            if (e.value < zero) {
                result = NumberT{-1.0};
            } else if (e.value > zero) {
                result = NumberT{1.0};
            } else {
                result = zero;
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fia_fi32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::sin(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::sin(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::sinh(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::sinh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::smoothstep(const type::Type* ty,
                                        utils::VectorRef<const constant::Value*> args,
                                        const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1,
                         const constant::Value* c2) {
        auto create = [&](auto low, auto high, auto x) -> ConstEval::Result {
            using NumberT = decltype(low);

            auto err = [&] {
                AddNote("when calculating smoothstep", source);
                return utils::Failure;
            };

            // t = clamp((x - low) / (high - low), 0.0, 1.0)
            auto x_minus_low = Sub(source, x, low);
            auto high_minus_low = Sub(source, high, low);
            if (!x_minus_low || !high_minus_low) {
                return err();
            }

            auto div = Div(source, x_minus_low.Get(), high_minus_low.Get());
            if (!div) {
                return err();
            }

            auto clamp = Clamp(source, div.Get(), NumberT(0), NumberT(1));
            auto t = clamp.Get();

            // result = t * t * (3.0 - 2.0 * t)
            auto t_times_t = Mul(source, t, t);
            auto t_times_2 = Mul(source, NumberT(2), t);
            if (!t_times_t || !t_times_2) {
                return err();
            }

            auto three_minus_t_times_2 = Sub(source, NumberT(3), t_times_2.Get());
            if (!three_minus_t_times_2) {
                return err();
            }

            auto result = Mul(source, t_times_t.Get(), three_minus_t_times_2.Get());
            if (!result) {
                return err();
            }
            return CreateScalar(source, c0->Type(), result.Get());
        };
        return Dispatch_fa_f32_f16(create, c0, c1, c2);
    };
    return TransformElements(builder, ty, transform, args[0], args[1], args[2]);
}

ConstEval::Result ConstEval::step(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0, const constant::Value* c1) {
        auto create = [&](auto edge, auto x) -> ConstEval::Result {
            using NumberT = decltype(edge);
            NumberT result = x.value < edge.value ? NumberT(0.0) : NumberT(1.0);
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformElements(builder, ty, transform, args[0], args[1]);
}

ConstEval::Result ConstEval::sqrt(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        return Dispatch_fa_f32_f16(SqrtFunc(source, c0->Type()), c0);
    };

    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::tan(const type::Type* ty,
                                 utils::VectorRef<const constant::Value*> args,
                                 const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::tan(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::tanh(const type::Type* ty,
                                  utils::VectorRef<const constant::Value*> args,
                                  const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) -> ConstEval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::tanh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::transpose(const type::Type* ty,
                                       utils::VectorRef<const constant::Value*> args,
                                       const Source&) {
    auto* m = args[0];
    auto* mat_ty = m->Type()->As<type::Matrix>();
    auto me = [&](size_t r, size_t c) { return m->Index(c)->Index(r); };
    auto* result_mat_ty = ty->As<type::Matrix>();

    // Produce column vectors from each row
    utils::Vector<const constant::Value*, 4> result_mat;
    for (size_t r = 0; r < mat_ty->rows(); ++r) {
        utils::Vector<const constant::Value*, 4> new_col_vec;
        for (size_t c = 0; c < mat_ty->columns(); ++c) {
            new_col_vec.Push(me(r, c));
        }
        result_mat.Push(
            builder.create<constant::Composite>(result_mat_ty->ColumnType(), new_col_vec));
    }
    return builder.create<constant::Composite>(ty, result_mat);
}

ConstEval::Result ConstEval::trunc(const type::Type* ty,
                                   utils::VectorRef<const constant::Value*> args,
                                   const Source& source) {
    auto transform = [&](const constant::Value* c0) {
        auto create = [&](auto i) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::trunc(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::unpack2x16float(const type::Type* ty,
                                             utils::VectorRef<const constant::Value*> args,
                                             const Source& source) {
    auto* inner_ty = type::Type::DeepestElementOf(ty);
    auto e = args[0]->ValueAs<u32>().value;

    utils::Vector<const constant::Value*, 2> els;
    els.Reserve(2);
    for (size_t i = 0; i < 2; ++i) {
        auto in = f16::FromBits(uint16_t((e >> (16 * i)) & 0x0000'ffff));
        auto val = CheckedConvert<f32>(in);
        if (!val) {
            AddError(OverflowErrorMessage(in, "f32"), source);
            if (use_runtime_semantics_) {
                val = f32(0.f);
            } else {
                return utils::Failure;
            }
        }
        auto el = CreateScalar(source, inner_ty, val.Get());
        if (!el) {
            return el;
        }
        els.Push(el.Get());
    }
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::unpack2x16snorm(const type::Type* ty,
                                             utils::VectorRef<const constant::Value*> args,
                                             const Source& source) {
    auto* inner_ty = type::Type::DeepestElementOf(ty);
    auto e = args[0]->ValueAs<u32>().value;

    utils::Vector<const constant::Value*, 2> els;
    els.Reserve(2);
    for (size_t i = 0; i < 2; ++i) {
        auto val = f32(
            std::max(static_cast<float>(int16_t((e >> (16 * i)) & 0x0000'ffff)) / 32767.f, -1.f));
        auto el = CreateScalar(source, inner_ty, val);
        if (!el) {
            return el;
        }
        els.Push(el.Get());
    }
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::unpack2x16unorm(const type::Type* ty,
                                             utils::VectorRef<const constant::Value*> args,
                                             const Source& source) {
    auto* inner_ty = type::Type::DeepestElementOf(ty);
    auto e = args[0]->ValueAs<u32>().value;

    utils::Vector<const constant::Value*, 2> els;
    els.Reserve(2);
    for (size_t i = 0; i < 2; ++i) {
        auto val = f32(static_cast<float>(uint16_t((e >> (16 * i)) & 0x0000'ffff)) / 65535.f);
        auto el = CreateScalar(source, inner_ty, val);
        if (!el) {
            return el;
        }
        els.Push(el.Get());
    }
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::unpack4x8snorm(const type::Type* ty,
                                            utils::VectorRef<const constant::Value*> args,
                                            const Source& source) {
    auto* inner_ty = type::Type::DeepestElementOf(ty);
    auto e = args[0]->ValueAs<u32>().value;

    utils::Vector<const constant::Value*, 4> els;
    els.Reserve(4);
    for (size_t i = 0; i < 4; ++i) {
        auto val =
            f32(std::max(static_cast<float>(int8_t((e >> (8 * i)) & 0x0000'00ff)) / 127.f, -1.f));
        auto el = CreateScalar(source, inner_ty, val);
        if (!el) {
            return el;
        }
        els.Push(el.Get());
    }
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::unpack4x8unorm(const type::Type* ty,
                                            utils::VectorRef<const constant::Value*> args,
                                            const Source& source) {
    auto* inner_ty = type::Type::DeepestElementOf(ty);
    auto e = args[0]->ValueAs<u32>().value;

    utils::Vector<const constant::Value*, 4> els;
    els.Reserve(4);
    for (size_t i = 0; i < 4; ++i) {
        auto val = f32(static_cast<float>(uint8_t((e >> (8 * i)) & 0x0000'00ff)) / 255.f);
        auto el = CreateScalar(source, inner_ty, val);
        if (!el) {
            return el;
        }
        els.Push(el.Get());
    }
    return builder.create<constant::Composite>(ty, std::move(els));
}

ConstEval::Result ConstEval::quantizeToF16(const type::Type* ty,
                                           utils::VectorRef<const constant::Value*> args,
                                           const Source& source) {
    auto transform = [&](const constant::Value* c) -> ConstEval::Result {
        auto value = c->ValueAs<f32>();
        auto conv = CheckedConvert<f32>(f16(value));
        if (!conv) {
            AddError(OverflowErrorMessage(value, "f16"), source);
            if (use_runtime_semantics_) {
                return ZeroValue(c->Type());
            } else {
                return utils::Failure;
            }
        }
        return CreateScalar(source, c->Type(), conv.Get());
    };
    return TransformElements(builder, ty, transform, args[0]);
}

ConstEval::Result ConstEval::Convert(const type::Type* target_ty,
                                     const constant::Value* value,
                                     const Source& source) {
    if (value->Type() == target_ty) {
        return value;
    }
    return ConvertInternal(value, builder, target_ty, source, use_runtime_semantics_);
}

void ConstEval::AddError(const std::string& msg, const Source& source) const {
    if (use_runtime_semantics_) {
        builder.Diagnostics().add_warning(diag::System::Resolver, msg, source);
    } else {
        builder.Diagnostics().add_error(diag::System::Resolver, msg, source);
    }
}

void ConstEval::AddWarning(const std::string& msg, const Source& source) const {
    builder.Diagnostics().add_warning(diag::System::Resolver, msg, source);
}

void ConstEval::AddNote(const std::string& msg, const Source& source) const {
    builder.Diagnostics().add_note(diag::System::Resolver, msg, source);
}

}  // namespace tint::resolver
