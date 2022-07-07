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

#include "src/tint/resolver/resolver.h"

#include <optional>

#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/transform.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {

namespace {

/// TypeDispatch is a helper for calling the function `f`, passing a single zero-value argument of
/// the C++ type that corresponds to the sem::Type `type`. For example, calling `TypeDispatch()`
/// with a type of `sem::I32*` will call the function f with a single argument of `i32(0)`.
/// @returns the value returned by calling `f`.
/// @note `type` must be a scalar or abstract numeric type. Other types will not call `f`, and will
/// return the zero-initialized value of the return type for `f`.
template <typename F>
auto TypeDispatch(const sem::Type* type, F&& f) {
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
                                std::vector<const Constant*> elements);

/// Element holds a single scalar or abstract-numeric value.
/// Element implements the Constant interface.
template <typename T>
struct Element : Constant {
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
    const Constant* Index(size_t) const override { return nullptr; }
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
        auto* res = TypeDispatch(target_ty, [&](auto zero_to) -> const Constant* {
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
                constexpr auto kInf = std::numeric_limits<double>::infinity();
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<Element<TO>>(target_ty, TO(-kInf));
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<Element<TO>>(target_ty, TO(kInf));
                }
            } else {
                // [x -> integer] - number not exactly representable
                // https://www.w3.org/TR/WGSL/#floating-point-conversion
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return builder.create<Element<TO>>(target_ty, TO(TO::kLowest));
                    case ConversionFailure::kExceedsPositiveLimit:
                        return builder.create<Element<TO>>(target_ty, TO(TO::kHighest));
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
    Splat(const sem::Type* t, const Constant* e, size_t n) : type(t), el(e), count(n) {}
    ~Splat() override = default;
    const sem::Type* Type() const override { return type; }
    std::variant<std::monostate, AInt, AFloat> Value() const override { return {}; }
    const Constant* Index(size_t i) const override { return i < count ? el : nullptr; }
    bool AllZero() const override { return el->AllZero(); }
    bool AnyZero() const override { return el->AnyZero(); }
    bool AllEqual() const override { return true; }
    size_t Hash() const override { return utils::Hash(type, el->Hash(), count); }

    utils::Result<const Constant*> Convert(ProgramBuilder& builder,
                                           const sem::Type* target_ty,
                                           const Source& source) const override {
        // Convert the single splatted element type.
        auto conv_el = el->Convert(builder, sem::Type::ElementOf(target_ty), source);
        if (!conv_el) {
            return utils::Failure;
        }
        if (!conv_el.Get()) {
            return nullptr;
        }
        return builder.create<Splat>(target_ty, conv_el.Get(), count);
    }

    sem::Type const* const type;
    const Constant* el;
    const size_t count;
};

/// Composite holds a number of mixed child Constant values.
/// Composite may be of a vector, matrix or array type.
/// If each element is the same type and value, then a Splat would be a more efficient constant
/// implementation. Use CreateComposite() to create the appropriate Constant type.
/// Composite implements the Constant interface.
struct Composite : Constant {
    Composite(const sem::Type* t, std::vector<const Constant*> els, bool all_0, bool any_0)
        : type(t), elements(std::move(els)), all_zero(all_0), any_zero(any_0), hash(CalcHash()) {}
    ~Composite() override = default;
    const sem::Type* Type() const override { return type; }
    std::variant<std::monostate, AInt, AFloat> Value() const override { return {}; }
    const Constant* Index(size_t i) const override {
        return i < elements.size() ? elements[i] : nullptr;
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
        std::vector<const Constant*> conv_els;
        conv_els.reserve(elements.size());
        for (auto* el : elements) {
            auto conv_el = el->Convert(builder, el_ty, source);
            if (!conv_el) {
                return utils::Failure;
            }
            if (!conv_el.Get()) {
                return nullptr;
            }
            conv_els.emplace_back(conv_el.Get());
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
    const std::vector<const Constant*> elements;
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
        [&](Default) -> const Constant* {
            return TypeDispatch(type, [&](auto zero) -> const Constant* {
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
                                std::vector<const Constant*> elements) {
    if (elements.size() == 0) {
        return nullptr;
    }
    bool any_zero = false;
    bool all_zero = true;
    bool all_equal = true;
    auto* first = elements.front();
    for (auto* el : elements) {
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
        return builder.create<Splat>(type, elements[0], elements.size());
    } else {
        return builder.create<Composite>(type, std::move(elements), all_zero, any_zero);
    }
}

}  // namespace

const sem::Constant* Resolver::EvaluateLiteralValue(const ast::LiteralExpression* literal,
                                                    const sem::Type* type) {
    return Switch(
        literal,
        [&](const ast::BoolLiteralExpression* lit) {
            return CreateElement(*builder_, type, lit->value);
        },
        [&](const ast::IntLiteralExpression* lit) -> const Constant* {
            switch (lit->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                    return CreateElement(*builder_, type, AInt(lit->value));
                case ast::IntLiteralExpression::Suffix::kI:
                    return CreateElement(*builder_, type, i32(lit->value));
                case ast::IntLiteralExpression::Suffix::kU:
                    return CreateElement(*builder_, type, u32(lit->value));
            }
            return nullptr;
        },
        [&](const ast::FloatLiteralExpression* lit) -> const Constant* {
            switch (lit->suffix) {
                case ast::FloatLiteralExpression::Suffix::kNone:
                    return CreateElement(*builder_, type, AFloat(lit->value));
                case ast::FloatLiteralExpression::Suffix::kF:
                    return CreateElement(*builder_, type, f32(lit->value));
                case ast::FloatLiteralExpression::Suffix::kH:
                    return CreateElement(*builder_, type, f16(lit->value));
            }
            return nullptr;
        });
}

const sem::Constant* Resolver::EvaluateCtorOrConvValue(
    const std::vector<const sem::Expression*>& args,
    const sem::Type* ty) {
    // For zero value init, return 0s
    if (args.empty()) {
        return ZeroValue(*builder_, ty);
    }

    uint32_t el_count = 0;
    auto* el_ty = sem::Type::ElementOf(ty, &el_count);
    if (!el_ty) {
        return nullptr;  // Target type does not support constant values
    }

    if (args.size() == 1) {
        // Type constructor or conversion that takes a single argument.
        auto& src = args[0]->Declaration()->source;
        auto* arg = static_cast<const Constant*>(args[0]->ConstantValue());
        if (!arg) {
            return nullptr;  // Single argument is not constant.
        }

        if (ty->is_scalar()) {  // Scalar type conversion: i32(x), u32(x), bool(x), etc
            return ConvertValue(arg, el_ty, src).Get();
        }

        if (arg->Type() == el_ty) {
            // Argument type matches function type. This is a splat.
            auto splat = [&](size_t n) { return builder_->create<Splat>(ty, arg, n); };
            return Switch(
                ty,  //
                [&](const sem::Vector* v) { return splat(v->Width()); },
                [&](const sem::Matrix* m) { return splat(m->columns()); },
                [&](const sem::Array* a) { return splat(a->Count()); });
        }

        // Argument type and function type mismatch. This is a type conversion.
        if (auto conv = ConvertValue(arg, ty, src)) {
            return conv.Get();
        }

        return nullptr;
    }

    // Multiple arguments. Must be a type constructor.

    std::vector<const Constant*> els;  // The constant elements for the composite constant.
    els.reserve(el_count);

    // Helper for pushing all the argument constants to `els`.
    auto push_all_args = [&] {
        for (auto* expr : args) {
            auto* arg = static_cast<const Constant*>(expr->ConstantValue());
            if (!arg) {
                return;
            }
            els.emplace_back(arg);
        }
    };

    // TODO(crbug.com/tint/1611): Add structure support.

    Switch(
        ty,  // What's the target type being constructed?
        [&](const sem::Vector*) {
            // Vector can be constructed with a mix of scalars / abstract numerics and smaller
            // vectors.
            for (auto* expr : args) {
                auto* arg = static_cast<const Constant*>(expr->ConstantValue());
                if (!arg) {
                    return;
                }
                auto* arg_ty = arg->Type();
                if (auto* arg_vec = arg_ty->As<sem::Vector>()) {
                    // Extract out vector elements.
                    for (uint32_t i = 0; i < arg_vec->Width(); i++) {
                        auto* el = static_cast<const Constant*>(arg->Index(i));
                        if (!el) {
                            return;
                        }
                        els.emplace_back(el);
                    }
                } else {
                    els.emplace_back(arg);
                }
            }
        },
        [&](const sem::Matrix* m) {
            // Matrix can be constructed with a set of scalars / abstract numerics, or column
            // vectors.
            if (args.size() == m->columns() * m->rows()) {
                // Matrix built from scalars / abstract numerics
                for (uint32_t c = 0; c < m->columns(); c++) {
                    std::vector<const Constant*> column;
                    column.reserve(m->rows());
                    for (uint32_t r = 0; r < m->rows(); r++) {
                        auto* arg =
                            static_cast<const Constant*>(args[r + c * m->rows()]->ConstantValue());
                        if (!arg) {
                            return;
                        }
                        column.emplace_back(arg);
                    }
                    els.push_back(CreateComposite(*builder_, m->ColumnType(), std::move(column)));
                }
            } else if (args.size() == m->columns()) {
                // Matrix built from column vectors
                push_all_args();
            }
        },
        [&](const sem::Array*) {
            // Arrays must be constructed using a list of elements
            push_all_args();
        });

    if (els.size() != el_count) {
        // If the number of constant elements doesn't match the type, then something went wrong.
        return nullptr;
    }
    // Construct and return either a Composite or Splat.
    return CreateComposite(*builder_, ty, std::move(els));
}

const sem::Constant* Resolver::EvaluateIndexValue(const sem::Expression* obj_expr,
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

const sem::Constant* Resolver::EvaluateSwizzleValue(const sem::Expression* vec_expr,
                                                    const sem::Type* type,
                                                    const std::vector<uint32_t>& indices) {
    auto* vec_val = vec_expr->ConstantValue();
    if (!vec_val) {
        return nullptr;
    }
    if (indices.size() == 1) {
        return static_cast<const Constant*>(vec_val->Index(indices[0]));
    } else {
        auto values = utils::Transform(
            indices, [&](uint32_t i) { return static_cast<const Constant*>(vec_val->Index(i)); });
        return CreateComposite(*builder_, type, std::move(values));
    }
}

const sem::Constant* Resolver::EvaluateBitcastValue(const sem::Expression*, const sem::Type*) {
    // TODO(crbug.com/tint/1581): Implement @const intrinsics
    return nullptr;
}

const sem::Constant* Resolver::EvaluateBinaryValue(const sem::Expression*,
                                                   const sem::Expression*,
                                                   const IntrinsicTable::BinaryOperator&) {
    // TODO(crbug.com/tint/1581): Implement @const intrinsics
    return nullptr;
}

const sem::Constant* Resolver::EvaluateUnaryValue(const sem::Expression*,
                                                  const IntrinsicTable::UnaryOperator&) {
    // TODO(crbug.com/tint/1581): Implement @const intrinsics
    return nullptr;
}

utils::Result<const sem::Constant*> Resolver::ConvertValue(const sem::Constant* value,
                                                           const sem::Type* target_ty,
                                                           const Source& source) {
    if (value->Type() == target_ty) {
        return value;
    }
    auto conv = static_cast<const Constant*>(value)->Convert(*builder_, target_ty, source);
    if (!conv) {
        return utils::Failure;
    }
    return conv.Get();
}

}  // namespace tint::resolver
