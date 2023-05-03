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

#include "src/tint/transform/builtin_polyfill.h"

#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/type_expression.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/switch.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/map.h"

using namespace tint::number_suffixes;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::transform::BuiltinPolyfill);
TINT_INSTANTIATE_TYPEINFO(tint::transform::BuiltinPolyfill::Config);

namespace tint::transform {

/// BinaryOpSignature is tuple of a binary op, LHS type and RHS type
using BinaryOpSignature = std::tuple<ast::BinaryOp, const type::Type*, const type::Type*>;

/// PIMPL state for the transform
struct BuiltinPolyfill::State {
    /// Constructor
    /// @param program the source program
    /// @param config the transform config
    State(const Program* program, const Config& config) : src(program), cfg(config) {
        has_full_ptr_params = false;
        for (auto* enable : src->AST().Enables()) {
            if (enable->HasExtension(builtin::Extension::kChromiumExperimentalFullPtrParameters)) {
                has_full_ptr_params = true;
                break;
            }
        }
    }

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    Transform::ApplyResult Run() {
        for (auto* node : src->ASTNodes().Objects()) {
            Switch(
                node,  //
                [&](const ast::CallExpression* expr) { Call(expr); },
                [&](const ast::BinaryExpression* bin_op) {
                    if (auto* s = src->Sem().Get(bin_op);
                        !s || s->Stage() == sem::EvaluationStage::kConstant ||
                        s->Stage() == sem::EvaluationStage::kNotEvaluated) {
                        return;  // Don't polyfill @const expressions
                    }
                    switch (bin_op->op) {
                        case ast::BinaryOp::kShiftLeft:
                        case ast::BinaryOp::kShiftRight: {
                            if (cfg.builtins.bitshift_modulo) {
                                ctx.Replace(bin_op,
                                            [this, bin_op] { return BitshiftModulo(bin_op); });
                                made_changes = true;
                            }
                            break;
                        }
                        case ast::BinaryOp::kDivide: {
                            if (cfg.builtins.int_div_mod) {
                                auto* lhs_ty = src->TypeOf(bin_op->lhs)->UnwrapRef();
                                if (lhs_ty->is_integer_scalar_or_vector()) {
                                    ctx.Replace(bin_op,
                                                [this, bin_op] { return IntDivMod(bin_op); });
                                    made_changes = true;
                                }
                            }
                            break;
                        }
                        case ast::BinaryOp::kModulo: {
                            if (cfg.builtins.int_div_mod) {
                                auto* lhs_ty = src->TypeOf(bin_op->lhs)->UnwrapRef();
                                if (lhs_ty->is_integer_scalar_or_vector()) {
                                    ctx.Replace(bin_op,
                                                [this, bin_op] { return IntDivMod(bin_op); });
                                    made_changes = true;
                                }
                            }
                            if (cfg.builtins.precise_float_mod) {
                                auto* lhs_ty = src->TypeOf(bin_op->lhs)->UnwrapRef();
                                if (lhs_ty->is_float_scalar_or_vector()) {
                                    ctx.Replace(bin_op,
                                                [this, bin_op] { return PreciseFloatMod(bin_op); });
                                    made_changes = true;
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                },
                [&](const ast::Expression* expr) {
                    if (cfg.builtins.bgra8unorm) {
                        if (auto* ty_expr = src->Sem().Get<sem::TypeExpression>(expr)) {
                            if (auto* tex = ty_expr->Type()->As<type::StorageTexture>()) {
                                if (tex->texel_format() == builtin::TexelFormat::kBgra8Unorm) {
                                    ctx.Replace(expr, [this, tex] {
                                        return ctx.dst->Expr(ctx.dst->ty.storage_texture(
                                            tex->dim(), builtin::TexelFormat::kRgba8Unorm,
                                            tex->access()));
                                    });
                                    made_changes = true;
                                }
                            }
                        }
                    }
                });
        }

        if (!made_changes) {
            return SkipTransform;
        }

        ctx.Clone();
        return Program(std::move(b));
    }

  private:
    /// The source program
    Program const* const src;
    /// The transform config
    const Config& cfg;
    /// The destination program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx{&b, src};
    /// The source clone context
    const sem::Info& sem = src->Sem();
    /// Polyfill functions for binary operators.
    utils::Hashmap<BinaryOpSignature, Symbol, 8> binary_op_polyfills;
    /// Polyfill builtins.
    utils::Hashmap<const sem::Builtin*, Symbol, 8> builtin_polyfills;
    /// Polyfill f32 conversion to i32 or u32 (or vectors of)
    utils::Hashmap<const type::Type*, Symbol, 2> f32_conv_polyfills;
    // Tracks whether the chromium_experimental_full_ptr_parameters extension has been enabled.
    bool has_full_ptr_params = false;
    /// True if the transform has made changes (i.e. the program needs cloning)
    bool made_changes = false;

    ////////////////////////////////////////////////////////////////////////////
    // Function polyfills
    ////////////////////////////////////////////////////////////////////////////

    /// Builds the polyfill function for the `acosh` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol acosh(const type::Type* ty) {
        auto name = b.Symbols().New("tint_acosh");
        uint32_t width = WidthOf(ty);

        auto V = [&](AFloat value) -> const ast::Expression* {
            const ast::Expression* expr = b.Expr(value);
            if (width == 1) {
                return expr;
            }
            return b.Call(T(ty), expr);
        };

        utils::Vector<const ast::Statement*, 4> body;
        switch (cfg.builtins.acosh) {
            case Level::kFull:
                // return log(x + sqrt(x*x - 1));
                body.Push(b.Return(
                    b.Call("log", b.Add("x", b.Call("sqrt", b.Sub(b.Mul("x", "x"), 1_a))))));
                break;
            case Level::kRangeCheck: {
                // return select(acosh(x), 0, x < 1);
                body.Push(b.Return(
                    b.Call("select", b.Call("acosh", "x"), V(0.0_a), b.LessThan("x", V(1.0_a)))));
                break;
            }
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(cfg.builtins.acosh);
                return {};
        }

        b.Func(name, utils::Vector{b.Param("x", T(ty))}, T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `asinh` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol asinh(const type::Type* ty) {
        auto name = b.Symbols().New("tint_sinh");

        // return log(x + sqrt(x*x + 1));
        b.Func(name, utils::Vector{b.Param("x", T(ty))}, T(ty),
               utils::Vector{
                   b.Return(b.Call("log", b.Add("x", b.Call("sqrt", b.Add(b.Mul("x", "x"), 1_a))))),
               });

        return name;
    }

    /// Builds the polyfill function for the `atanh` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol atanh(const type::Type* ty) {
        auto name = b.Symbols().New("tint_atanh");
        uint32_t width = WidthOf(ty);

        auto V = [&](AFloat value) -> const ast::Expression* {
            const ast::Expression* expr = b.Expr(value);
            if (width == 1) {
                return expr;
            }
            return b.Call(T(ty), expr);
        };

        utils::Vector<const ast::Statement*, 1> body;
        switch (cfg.builtins.atanh) {
            case Level::kFull:
                // return log((1+x) / (1-x)) * 0.5
                body.Push(
                    b.Return(b.Mul(b.Call("log", b.Div(b.Add(1_a, "x"), b.Sub(1_a, "x"))), 0.5_a)));
                break;
            case Level::kRangeCheck:
                // return select(atanh(x), 0, x >= 1);
                body.Push(b.Return(b.Call("select", b.Call("atanh", "x"), V(0.0_a),
                                          b.GreaterThanEqual("x", V(1.0_a)))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(cfg.builtins.acosh);
                return {};
        }

        b.Func(name, utils::Vector{b.Param("x", T(ty))}, T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `clamp` builtin when called with integer arguments
    /// (scalar or vector)
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol clampInteger(const type::Type* ty) {
        auto name = b.Symbols().New("tint_clamp");

        b.Func(name,
               utils::Vector{
                   b.Param("e", T(ty)),
                   b.Param("low", T(ty)),
                   b.Param("high", T(ty)),
               },
               T(ty),
               utils::Vector{
                   // return min(max(e, low), high);
                   b.Return(b.Call("min", b.Call("max", "e", "low"), "high")),
               });
        return name;
    }

    /// Builds the polyfill function for the `countLeadingZeros` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol countLeadingZeros(const type::Type* ty) {
        auto name = b.Symbols().New("tint_count_leading_zeros");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() {
            if (width == 1) {
                return b.ty.u32();
            }
            return b.ty.vec<u32>(width);
        };
        auto V = [&](uint32_t value) -> const ast::Expression* {
            return ScalarOrVector(width, u32(value));
        };
        b.Func(
            name,
            utils::Vector{
                b.Param("v", T(ty)),
            },
            T(ty),
            utils::Vector{
                // var x = U(v);
                b.Decl(b.Var("x", b.Call(U(), b.Expr("v")))),
                // let b16 = select(0, 16, x <= 0x0000ffff);
                b.Decl(b.Let("b16",
                             b.Call("select", V(0), V(16), b.LessThanEqual("x", V(0x0000ffff))))),
                // x = x << b16;
                b.Assign("x", b.Shl("x", "b16")),
                // let b8  = select(0, 8,  x <= 0x00ffffff);
                b.Decl(
                    b.Let("b8", b.Call("select", V(0), V(8), b.LessThanEqual("x", V(0x00ffffff))))),
                // x = x << b8;
                b.Assign("x", b.Shl("x", "b8")),
                // let b4  = select(0, 4,  x <= 0x0fffffff);
                b.Decl(
                    b.Let("b4", b.Call("select", V(0), V(4), b.LessThanEqual("x", V(0x0fffffff))))),
                // x = x << b4;
                b.Assign("x", b.Shl("x", "b4")),
                // let b2  = select(0, 2,  x <= 0x3fffffff);
                b.Decl(
                    b.Let("b2", b.Call("select", V(0), V(2), b.LessThanEqual("x", V(0x3fffffff))))),
                // x = x << b2;
                b.Assign("x", b.Shl("x", "b2")),
                // let b1  = select(0, 1,  x <= 0x7fffffff);
                b.Decl(
                    b.Let("b1", b.Call("select", V(0), V(1), b.LessThanEqual("x", V(0x7fffffff))))),
                // let is_zero  = select(0, 1, x == 0);
                b.Decl(b.Let("is_zero", b.Call("select", V(0), V(1), b.Equal("x", V(0))))),
                // return R((b16 | b8 | b4 | b2 | b1) + zero);
                b.Return(b.Call(T(ty), b.Add(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"),
                                             "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `countTrailingZeros` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol countTrailingZeros(const type::Type* ty) {
        auto name = b.Symbols().New("tint_count_trailing_zeros");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() {
            if (width == 1) {
                return b.ty.u32();
            }
            return b.ty.vec<u32>(width);
        };
        auto V = [&](uint32_t value) -> const ast::Expression* {
            return ScalarOrVector(width, u32(value));
        };
        auto B = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return b.Call<bool>(value);
            }
            return b.Call(b.ty.vec<bool>(width), value);
        };
        b.Func(
            name,
            utils::Vector{
                b.Param("v", T(ty)),
            },
            T(ty),
            utils::Vector{
                // var x = U(v);
                b.Decl(b.Var("x", b.Call(U(), b.Expr("v")))),
                // let b16 = select(16, 0, bool(x & 0x0000ffff));
                b.Decl(b.Let("b16", b.Call("select", V(16), V(0), B(b.And("x", V(0x0000ffff)))))),
                // x = x >> b16;
                b.Assign("x", b.Shr("x", "b16")),
                // let b8  = select(8,  0, bool(x & 0x000000ff));
                b.Decl(b.Let("b8", b.Call("select", V(8), V(0), B(b.And("x", V(0x000000ff)))))),
                // x = x >> b8;
                b.Assign("x", b.Shr("x", "b8")),
                // let b4  = select(4,  0, bool(x & 0x0000000f));
                b.Decl(b.Let("b4", b.Call("select", V(4), V(0), B(b.And("x", V(0x0000000f)))))),
                // x = x >> b4;
                b.Assign("x", b.Shr("x", "b4")),
                // let b2  = select(2,  0, bool(x & 0x00000003));
                b.Decl(b.Let("b2", b.Call("select", V(2), V(0), B(b.And("x", V(0x00000003)))))),
                // x = x >> b2;
                b.Assign("x", b.Shr("x", "b2")),
                // let b1  = select(1,  0, bool(x & 0x00000001));
                b.Decl(b.Let("b1", b.Call("select", V(1), V(0), B(b.And("x", V(0x00000001)))))),
                // let is_zero  = select(0, 1, x == 0);
                b.Decl(b.Let("is_zero", b.Call("select", V(0), V(1), b.Equal("x", V(0))))),
                // return R((b16 | b8 | b4 | b2 | b1) + zero);
                b.Return(b.Call(T(ty), b.Add(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"),
                                             "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `extractBits` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol extractBits(const type::Type* ty) {
        auto name = b.Symbols().New("tint_extract_bits");
        uint32_t width = WidthOf(ty);

        constexpr uint32_t W = 32u;  // 32-bit

        auto vecN_u32 = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return value;
            }
            return b.Call(b.ty.vec<u32>(width), value);
        };

        utils::Vector<const ast::Statement*, 8> body{
            b.Decl(b.Let("s", b.Call("min", "offset", u32(W)))),
            b.Decl(b.Let("e", b.Call("min", u32(W), b.Add("s", "count")))),
        };

        switch (cfg.builtins.extract_bits) {
            case Level::kFull:
                body.Push(b.Decl(b.Let("shl", b.Sub(u32(W), "e"))));
                body.Push(b.Decl(b.Let("shr", b.Add("shl", "s"))));
                // Here we don't want the shl and shr modulos the rhs, so handle the `rhs >= 32u`
                // cases using `select`. In order to handle the signed shr `lhs >> rhs` corrently,
                // use `(lhs >> 31u) >> 1u` if `rhs >= 32u`.
                body.Push(b.Decl(b.Let("shl_result", b.Call("select", b.Call(T(ty)),
                                                            b.Shl("v", vecN_u32(b.Expr("shl"))),
                                                            b.LessThan("shl", 32_u)))));
                body.Push(b.Return(b.Call(
                    "select",
                    b.Shr(b.Shr("shl_result", vecN_u32(b.Expr(31_u))), vecN_u32(b.Expr(1_u))),
                    b.Shr("shl_result", vecN_u32(b.Expr("shr"))), b.LessThan("shr", 32_u))

                                       ));
                break;
            case Level::kClampParameters:
                body.Push(b.Return(b.Call("extractBits", "v", "s", b.Sub("e", "s"))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(cfg.builtins.extract_bits);
                return {};
        }

        b.Func(name,
               utils::Vector{
                   b.Param("v", T(ty)),
                   b.Param("offset", b.ty.u32()),
                   b.Param("count", b.ty.u32()),
               },
               T(ty), std::move(body));

        return name;
    }

    /// Builds the polyfill function for the `firstLeadingBit` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol firstLeadingBit(const type::Type* ty) {
        auto name = b.Symbols().New("tint_first_leading_bit");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() {
            if (width == 1) {
                return b.ty.u32();
            }
            return b.ty.vec<u32>(width);
        };
        auto V = [&](uint32_t value) -> const ast::Expression* {
            return ScalarOrVector(width, u32(value));
        };
        auto B = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return b.Call<bool>(value);
            }
            return b.Call(b.ty.vec<bool>(width), value);
        };

        const ast::Expression* x = nullptr;
        if (ty->is_unsigned_integer_scalar_or_vector()) {
            x = b.Expr("v");
        } else {
            // If ty is signed, then the value is inverted if the sign is negative
            x = b.Call("select",                        //
                       b.Call(U(), "v"),                //
                       b.Call(U(), b.Complement("v")),  //
                       b.LessThan("v", ScalarOrVector(width, 0_i)));
        }

        b.Func(
            name,
            utils::Vector{
                b.Param("v", T(ty)),
            },
            T(ty),
            utils::Vector{
                // var x = v;                          (unsigned)
                // var x = select(U(v), ~U(v), v < 0); (signed)
                b.Decl(b.Var("x", x)),
                // let b16 = select(0, 16, bool(x & 0xffff0000));
                b.Decl(b.Let("b16", b.Call("select", V(0), V(16), B(b.And("x", V(0xffff0000)))))),
                // x = x >> b16;
                b.Assign("x", b.Shr("x", "b16")),
                // let b8  = select(0, 8,  bool(x & 0x0000ff00));
                b.Decl(b.Let("b8", b.Call("select", V(0), V(8), B(b.And("x", V(0x0000ff00)))))),
                // x = x >> b8;
                b.Assign("x", b.Shr("x", "b8")),
                // let b4  = select(0, 4,  bool(x & 0x000000f0));
                b.Decl(b.Let("b4", b.Call("select", V(0), V(4), B(b.And("x", V(0x000000f0)))))),
                // x = x >> b4;
                b.Assign("x", b.Shr("x", "b4")),
                // let b2  = select(0, 2,  bool(x & 0x0000000c));
                b.Decl(b.Let("b2", b.Call("select", V(0), V(2), B(b.And("x", V(0x0000000c)))))),
                // x = x >> b2;
                b.Assign("x", b.Shr("x", "b2")),
                // let b1  = select(0, 1,  bool(x & 0x00000002));
                b.Decl(b.Let("b1", b.Call("select", V(0), V(1), B(b.And("x", V(0x00000002)))))),
                // let is_zero  = select(0, 0xffffffff, x == 0);
                b.Decl(b.Let("is_zero", b.Call("select", V(0), V(0xffffffff), b.Equal("x", V(0))))),
                // return R(b16 | b8 | b4 | b2 | b1 | zero);
                b.Return(b.Call(
                    T(ty), b.Or(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"), "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `firstTrailingBit` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol firstTrailingBit(const type::Type* ty) {
        auto name = b.Symbols().New("tint_first_trailing_bit");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() {
            if (width == 1) {
                return b.ty.u32();
            }
            return b.ty.vec<u32>(width);
        };
        auto V = [&](uint32_t value) -> const ast::Expression* {
            return ScalarOrVector(width, u32(value));
        };
        auto B = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return b.Call<bool>(value);
            }
            return b.Call(b.ty.vec<bool>(width), value);
        };
        b.Func(
            name,
            utils::Vector{
                b.Param("v", T(ty)),
            },
            T(ty),
            utils::Vector{
                // var x = U(v);
                b.Decl(b.Var("x", b.Call(U(), b.Expr("v")))),
                // let b16 = select(16, 0, bool(x & 0x0000ffff));
                b.Decl(b.Let("b16", b.Call("select", V(16), V(0), B(b.And("x", V(0x0000ffff)))))),
                // x = x >> b16;
                b.Assign("x", b.Shr("x", "b16")),
                // let b8  = select(8,  0, bool(x & 0x000000ff));
                b.Decl(b.Let("b8", b.Call("select", V(8), V(0), B(b.And("x", V(0x000000ff)))))),
                // x = x >> b8;
                b.Assign("x", b.Shr("x", "b8")),
                // let b4  = select(4,  0, bool(x & 0x0000000f));
                b.Decl(b.Let("b4", b.Call("select", V(4), V(0), B(b.And("x", V(0x0000000f)))))),
                // x = x >> b4;
                b.Assign("x", b.Shr("x", "b4")),
                // let b2  = select(2,  0, bool(x & 0x00000003));
                b.Decl(b.Let("b2", b.Call("select", V(2), V(0), B(b.And("x", V(0x00000003)))))),
                // x = x >> b2;
                b.Assign("x", b.Shr("x", "b2")),
                // let b1  = select(1,  0, bool(x & 0x00000001));
                b.Decl(b.Let("b1", b.Call("select", V(1), V(0), B(b.And("x", V(0x00000001)))))),
                // let is_zero  = select(0, 0xffffffff, x == 0);
                b.Decl(b.Let("is_zero", b.Call("select", V(0), V(0xffffffff), b.Equal("x", V(0))))),
                // return R(b16 | b8 | b4 | b2 | b1 | is_zero);
                b.Return(b.Call(
                    T(ty), b.Or(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"), "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `insertBits` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol insertBits(const type::Type* ty) {
        auto name = b.Symbols().New("tint_insert_bits");
        uint32_t width = WidthOf(ty);

        // Currently in WGSL parameters of insertBits must be i32, u32, vecN<i32> or vecN<u32>
        if (TINT_UNLIKELY(((!type::Type::DeepestElementOf(ty)->IsAnyOf<type::I32, type::U32>())))) {
            TINT_ICE(Transform, b.Diagnostics())
                << "insertBits polyfill only support i32, u32, and vector of i32 or u32, got "
                << ty->FriendlyName();
            return {};
        }

        constexpr uint32_t W = 32u;  // 32-bit

        auto V = [&](auto value) -> const ast::Expression* {
            const ast::Expression* expr = b.Expr(value);
            if (!ty->is_unsigned_integer_scalar_or_vector()) {
                expr = b.Call<i32>(expr);
            }
            if (ty->Is<type::Vector>()) {
                expr = b.Call(T(ty), expr);
            }
            return expr;
        };
        auto U = [&](auto value) -> const ast::Expression* {
            if (width == 1) {
                return b.Expr(value);
            }
            return b.vec(b.ty.u32(), width, value);
        };

        // Polyfill algorithm:
        //      s = min(offset, 32u);
        //      e = min(32u, (s + count));
        //      mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
        //      return (((n << s) & mask) | (v & ~(mask)));
        // Note that the algorithm above use the left-shifting in C++ manner, but in WGSL, HLSL, MSL
        // the rhs are modulo to bit-width of lhs (that is 32u in this case), and in GLSL the result
        // is undefined if rhs is greater than or equal to bit-width of lhs. The results of `x << y`
        // in C++ and HLSL are different when `y >= 32u`, and the `s` and `e` defined above can be
        // 32u, which are cases we must handle specially. Replace all `(x << y)` to
        // `select(Tx(), x << y, y < 32u)`, in which `Tx` is the type of x, where y can be greater
        // than or equal to 32u.
        // WGSL polyfill function:
        //      fn tint_insert_bits(v : T, n : T, offset : u32, count : u32) -> T {
        //          let e = offset + count;
        //          let mask = (
        //                        (select(0u, 1u << offset, offset < 32u) - 1u) ^
        //                        (select(0u, 1u << e, e < 32u) - 1u)
        //                     );
        //          return ((select(T(), n << offset, offset < 32u) & mask) | (v & ~(mask)));
        //      }

        utils::Vector<const ast::Statement*, 8> body;

        switch (cfg.builtins.insert_bits) {
            case Level::kFull:
                // let e = offset + count;
                body.Push(b.Decl(b.Let("e", b.Add("offset", "count"))));

                // let mask = (
                //              (select(0u, 1u << offset, offset < 32u) - 1u) ^
                //              (select(0u, 1u << e, e < 32u) - 1u)
                //            );
                body.Push(b.Decl(b.Let(
                    "mask",
                    b.Xor(  //
                        b.Sub(
                            b.Call("select", 0_u, b.Shl(1_u, "offset"), b.LessThan("offset", 32_u)),
                            1_u),
                        b.Sub(b.Call("select", 0_u, b.Shl(1_u, "e"), b.LessThan("e", 32_u)),
                              1_u)  //
                        ))));

                // return ((select(T(), n << offset, offset < 32u) & mask) | (v & ~(mask)));
                body.Push(
                    b.Return(b.Or(b.And(b.Call("select", b.Call(T(ty)), b.Shl("n", U("offset")),
                                               b.LessThan("offset", 32_u)),
                                        V("mask")),
                                  b.And("v", V(b.Complement("mask"))))));

                break;
            case Level::kClampParameters:
                body.Push(b.Decl(b.Let("s", b.Call("min", "offset", u32(W)))));
                body.Push(b.Decl(b.Let("e", b.Call("min", u32(W), b.Add("s", "count")))));
                body.Push(b.Return(b.Call("insertBits", "v", "n", "s", b.Sub("e", "s"))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(cfg.builtins.insert_bits);
                return {};
        }

        b.Func(name,
               utils::Vector{
                   b.Param("v", T(ty)),
                   b.Param("n", T(ty)),
                   b.Param("offset", b.ty.u32()),
                   b.Param("count", b.ty.u32()),
               },
               T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `reflect` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol reflect(const type::Type* ty) {
        auto name = b.Symbols().New("tint_reflect");

        // WGSL polyfill function:
        //      fn tint_reflect(e1 : T, e2 : T) -> T {
        //          let factor = (-2.0 * dot(e1, e2));
        //          return (e1 + (factor * e2));
        //      }
        // Using -2.0 instead of 2.0 in factor to prevent the optimization that cause wrong result.
        // See https://crbug.com/tint/1798 for more details.
        auto body = utils::Vector{
            b.Decl(b.Let("factor", b.Mul(-2.0_a, b.Call("dot", "e1", "e2")))),
            b.Return(b.Add("e1", b.Mul("factor", "e2"))),
        };
        b.Func(name,
               utils::Vector{
                   b.Param("e1", T(ty)),
                   b.Param("e2", T(ty)),
               },
               T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `saturate` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol saturate(const type::Type* ty) {
        auto name = b.Symbols().New("tint_saturate");
        auto body = utils::Vector{
            b.Return(b.Call("clamp", "v", b.Call(T(ty), 0_a), b.Call(T(ty), 1_a))),
        };
        b.Func(name,
               utils::Vector{
                   b.Param("v", T(ty)),
               },
               T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `sign` builtin when the element type is integer
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol sign_int(const type::Type* ty) {
        const uint32_t width = WidthOf(ty);
        auto zero = [&] { return ScalarOrVector(width, 0_a); };

        // pos_or_neg_one = (v > 0) ? 1 : -1
        auto pos_or_neg_one = b.Call("select",                     //
                                     ScalarOrVector(width, -1_a),  //
                                     ScalarOrVector(width, 1_a),   //
                                     b.GreaterThan("v", zero()));

        auto name = b.Symbols().New("tint_sign");
        b.Func(name,
               utils::Vector{
                   b.Param("v", T(ty)),
               },
               T(ty),
               utils::Vector{
                   b.Return(b.Call("select", pos_or_neg_one, zero(), b.Equal("v", zero()))),
               });

        return name;
    }

    /// Builds the polyfill function for the `textureSampleBaseClampToEdge` builtin, when the
    /// texture type is texture_2d<f32>.
    /// @return the polyfill function name
    Symbol textureSampleBaseClampToEdge_2d_f32() {
        auto name = b.Symbols().New("tint_textureSampleBaseClampToEdge");
        auto body = utils::Vector{
            b.Decl(b.Let("dims", b.Call(b.ty.vec2<f32>(), b.Call("textureDimensions", "t", 0_a)))),
            b.Decl(b.Let("half_texel", b.Div(b.vec2<f32>(0.5_a), "dims"))),
            b.Decl(
                b.Let("clamped", b.Call("clamp", "coord", "half_texel", b.Sub(1_a, "half_texel")))),
            b.Return(b.Call("textureSampleLevel", "t", "s", "clamped", 0_a)),
        };
        b.Func(name,
               utils::Vector{
                   b.Param("t", b.ty.sampled_texture(type::TextureDimension::k2d, b.ty.f32())),
                   b.Param("s", b.ty.sampler(type::SamplerKind::kSampler)),
                   b.Param("coord", b.ty.vec2<f32>()),
               },
               b.ty.vec4<f32>(), body);
        return name;
    }

    /// Builds the polyfill function for the `quantizeToF16` builtin, by replacing the vector form
    /// with scalar calls.
    /// @param vec the vector type
    /// @return the polyfill function name
    Symbol quantizeToF16(const type::Vector* vec) {
        auto name = b.Symbols().New("tint_quantizeToF16");
        utils::Vector<const ast::Expression*, 4> args;
        for (uint32_t i = 0; i < vec->Width(); i++) {
            args.Push(b.Call("quantizeToF16", b.IndexAccessor("v", u32(i))));
        }
        b.Func(name,
               utils::Vector{
                   b.Param("v", T(vec)),
               },
               T(vec),
               utils::Vector{
                   b.Return(b.Call(T(vec), std::move(args))),
               });
        return name;
    }

    /// Builds the polyfill function for the `workgroupUniformLoad` builtin.
    /// @param type the type being loaded
    /// @return the polyfill function name
    Symbol workgroupUniformLoad(const type::Type* type) {
        if (!has_full_ptr_params) {
            b.Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
            has_full_ptr_params = true;
        }
        auto name = b.Symbols().New("tint_workgroupUniformLoad");
        b.Func(name,
               utils::Vector{
                   b.Param("p", b.ty.pointer(T(type), builtin::AddressSpace::kWorkgroup)),
               },
               T(type),
               utils::Vector{
                   b.CallStmt(b.Call("workgroupBarrier")),
                   b.Decl(b.Let("result", b.Deref("p"))),
                   b.CallStmt(b.Call("workgroupBarrier")),
                   b.Return("result"),
               });
        return name;
    }

    /// Builds the polyfill function to value convert a scalar or vector of f32 to an i32 or u32 (or
    /// vector of).
    /// @param source the type of the value being converted
    /// @param target the target conversion type
    /// @return the polyfill function name
    Symbol ConvF32ToIU32(const type::Type* source, const type::Type* target) {
        struct Limits {
            AFloat low_condition;
            AInt low_limit;
            AFloat high_condition;
            AInt high_limit;
        };
        const bool is_signed = target->is_signed_integer_scalar_or_vector();
        const Limits limits = is_signed ? Limits{
                                              /* low_condition   */ -AFloat(0x80000000),
                                              /* low_limit  */ -AInt(0x80000000),
                                              /* high_condition  */ AFloat(0x7fffff80),
                                              /* high_limit */ AInt(0x7fffffff),
                                          }
                                        : Limits{
                                              /* low_condition   */ AFloat(0),
                                              /* low_limit  */ AInt(0),
                                              /* high_condition  */ AFloat(0xffffff00),
                                              /* high_limit */ AInt(0xffffffff),
                                          };

        const uint32_t width = WidthOf(target);

        // select(target(v), low_limit, v < low_condition)
        auto* select_low = b.Call(builtin::Function::kSelect,               //
                                  b.Call(T(target), "v"),                   //
                                  ScalarOrVector(width, limits.low_limit),  //
                                  b.LessThan("v", ScalarOrVector(width, limits.low_condition)));

        // select(high_limit, select_low, v < high_condition)
        auto* select_high = b.Call(builtin::Function::kSelect,                //
                                   ScalarOrVector(width, limits.high_limit),  //
                                   select_low,                                //
                                   b.LessThan("v", ScalarOrVector(width, limits.high_condition)));

        auto name = b.Symbols().New(is_signed ? "tint_ftoi" : "tint_ftou");
        b.Func(name, utils::Vector{b.Param("v", T(source))}, T(target),
               utils::Vector{b.Return(select_high)});
        return name;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Inline polyfills
    ////////////////////////////////////////////////////////////////////////////

    /// Builds the polyfill inline expression for a bitshift left or bitshift right, ensuring that
    /// the RHS is modulo the bit-width of the LHS.
    /// @param bin_op the original BinaryExpression
    /// @return the polyfill value for bitshift operation
    const ast::Expression* BitshiftModulo(const ast::BinaryExpression* bin_op) {
        auto* lhs_ty = src->TypeOf(bin_op->lhs)->UnwrapRef();
        auto* rhs_ty = src->TypeOf(bin_op->rhs)->UnwrapRef();
        auto* lhs_el_ty = type::Type::DeepestElementOf(lhs_ty);
        const ast::Expression* mask = b.Expr(AInt(lhs_el_ty->Size() * 8 - 1));
        if (rhs_ty->Is<type::Vector>()) {
            mask = b.Call(CreateASTTypeFor(ctx, rhs_ty), mask);
        }
        auto* lhs = ctx.Clone(bin_op->lhs);
        auto* rhs = b.And(ctx.Clone(bin_op->rhs), mask);
        return b.create<ast::BinaryExpression>(ctx.Clone(bin_op->source), bin_op->op, lhs, rhs);
    }

    /// Builds the polyfill inline expression for a integer divide or modulo, preventing DBZs and
    /// integer overflows.
    /// @param bin_op the original BinaryExpression
    /// @return the polyfill divide or modulo
    const ast::Expression* IntDivMod(const ast::BinaryExpression* bin_op) {
        auto* lhs_ty = src->TypeOf(bin_op->lhs)->UnwrapRef();
        auto* rhs_ty = src->TypeOf(bin_op->rhs)->UnwrapRef();
        BinaryOpSignature sig{bin_op->op, lhs_ty, rhs_ty};
        auto fn = binary_op_polyfills.GetOrCreate(sig, [&] {
            const bool is_div = bin_op->op == ast::BinaryOp::kDivide;

            uint32_t lhs_width = 1;
            uint32_t rhs_width = 1;
            const auto* lhs_el_ty = type::Type::ElementOf(lhs_ty, &lhs_width);
            const auto* rhs_el_ty = type::Type::ElementOf(rhs_ty, &rhs_width);

            const uint32_t width = std::max(lhs_width, rhs_width);

            const char* lhs = "lhs";
            const char* rhs = "rhs";

            utils::Vector<const ast::Statement*, 4> body;

            if (lhs_width < width) {
                // lhs is scalar, rhs is vector. Convert lhs to vector.
                body.Push(b.Decl(b.Let("l", b.vec(T(lhs_el_ty), width, b.Expr(lhs)))));
                lhs = "l";
            }
            if (rhs_width < width) {
                // lhs is vector, rhs is scalar. Convert rhs to vector.
                body.Push(b.Decl(b.Let("r", b.vec(T(rhs_el_ty), width, b.Expr(rhs)))));
                rhs = "r";
            }

            auto name = b.Symbols().New(is_div ? "tint_div" : "tint_mod");

            auto* rhs_is_zero = b.Equal(rhs, ScalarOrVector(width, 0_a));

            if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                const auto bits = lhs_el_ty->Size() * 8;
                auto min_int = AInt(AInt::kLowestValue >> (AInt::kNumBits - bits));
                const ast::Expression* lhs_is_min = b.Equal(lhs, ScalarOrVector(width, min_int));
                const ast::Expression* rhs_is_minus_one = b.Equal(rhs, ScalarOrVector(width, -1_a));
                // use_one = rhs_is_zero | ((lhs == MIN_INT) & (rhs == -1))
                auto* use_one = b.Or(rhs_is_zero, b.And(lhs_is_min, rhs_is_minus_one));

                // Special handling for mod in case either operand is negative, as negative operands
                // for % is undefined behaviour for most backends (HLSL, MSL, GLSL, SPIR-V).
                if (!is_div) {
                    const char* rhs_or_one = "rhs_or_one";
                    body.Push(b.Decl(b.Let(
                        rhs_or_one, b.Call("select", rhs, ScalarOrVector(width, 1_a), use_one))));

                    // Is either operand negative?
                    // (lhs | rhs) & (1<<31)
                    auto sign_bit_mask = ScalarOrVector(width, u32(1 << (bits - 1)));
                    auto* lhs_or_rhs = CastScalarOrVector<u32>(width, b.Or(lhs, rhs_or_one));
                    auto* lhs_or_rhs_is_neg =
                        b.NotEqual(b.And(lhs_or_rhs, sign_bit_mask), ScalarOrVector(width, 0_u));

                    // lhs - trunc(lhs / rhs) * rhs (note: integral division truncates)
                    auto* slow_mod = b.Sub(lhs, b.Mul(b.Div(lhs, rhs_or_one), rhs_or_one));

                    // lhs % rhs
                    auto* fast_mod = b.Mod(lhs, rhs_or_one);

                    auto* use_slow = b.Call("any", lhs_or_rhs_is_neg);

                    body.Push(b.If(use_slow, b.Block(b.Return(slow_mod)),
                                   b.Else(b.Block(b.Return(fast_mod)))));

                } else {
                    auto* rhs_or_one = b.Call("select", rhs, ScalarOrVector(width, 1_a), use_one);
                    body.Push(b.Return(is_div ? b.Div(lhs, rhs_or_one) : b.Mod(lhs, rhs_or_one)));
                }

            } else {
                auto* rhs_or_one = b.Call("select", rhs, ScalarOrVector(width, 1_a), rhs_is_zero);
                body.Push(b.Return(is_div ? b.Div(lhs, rhs_or_one) : b.Mod(lhs, rhs_or_one)));
            }

            b.Func(name,
                   utils::Vector{
                       b.Param("lhs", T(lhs_ty)),
                       b.Param("rhs", T(rhs_ty)),
                   },
                   width == 1 ? T(lhs_ty) : b.ty.vec(T(lhs_el_ty), width),  // return type
                   std::move(body));

            return name;
        });
        auto* lhs = ctx.Clone(bin_op->lhs);
        auto* rhs = ctx.Clone(bin_op->rhs);
        return b.Call(fn, lhs, rhs);
    }

    /// Builds the polyfill inline expression for a precise float modulo, as defined in the spec.
    /// @param bin_op the original BinaryExpression
    /// @return the polyfill divide or modulo
    const ast::Expression* PreciseFloatMod(const ast::BinaryExpression* bin_op) {
        auto* lhs_ty = src->TypeOf(bin_op->lhs)->UnwrapRef();
        auto* rhs_ty = src->TypeOf(bin_op->rhs)->UnwrapRef();
        BinaryOpSignature sig{bin_op->op, lhs_ty, rhs_ty};
        auto fn = binary_op_polyfills.GetOrCreate(sig, [&] {
            uint32_t lhs_width = 1;
            uint32_t rhs_width = 1;
            const auto* lhs_el_ty = type::Type::ElementOf(lhs_ty, &lhs_width);
            const auto* rhs_el_ty = type::Type::ElementOf(rhs_ty, &rhs_width);

            const uint32_t width = std::max(lhs_width, rhs_width);

            const char* lhs = "lhs";
            const char* rhs = "rhs";

            utils::Vector<const ast::Statement*, 4> body;

            if (lhs_width < width) {
                // lhs is scalar, rhs is vector. Convert lhs to vector.
                body.Push(b.Decl(b.Let("l", b.vec(T(lhs_el_ty), width, b.Expr(lhs)))));
                lhs = "l";
            }
            if (rhs_width < width) {
                // lhs is vector, rhs is scalar. Convert rhs to vector.
                body.Push(b.Decl(b.Let("r", b.vec(T(rhs_el_ty), width, b.Expr(rhs)))));
                rhs = "r";
            }

            auto name = b.Symbols().New("tint_float_mod");

            // lhs - trunc(lhs / rhs) * rhs
            auto* precise_mod = b.Sub(lhs, b.Mul(b.Call("trunc", b.Div(lhs, rhs)), rhs));
            body.Push(b.Return(precise_mod));

            b.Func(name,
                   utils::Vector{
                       b.Param("lhs", T(lhs_ty)),
                       b.Param("rhs", T(rhs_ty)),
                   },
                   width == 1 ? T(lhs_ty) : b.ty.vec(T(lhs_el_ty), width),  // return type
                   std::move(body));

            return name;
        });
        auto* lhs = ctx.Clone(bin_op->lhs);
        auto* rhs = ctx.Clone(bin_op->rhs);
        return b.Call(fn, lhs, rhs);
    }

    /// @returns the AST type for the given sem type
    ast::Type T(const type::Type* ty) { return CreateASTTypeFor(ctx, ty); }

    /// @returns 1 if `ty` is not a vector, otherwise the vector width
    uint32_t WidthOf(const type::Type* ty) const {
        if (auto* v = ty->As<type::Vector>()) {
            return v->Width();
        }
        return 1;
    }

    /// @returns a scalar or vector with the given width, with each element with
    /// the given value.
    template <typename T>
    const ast::Expression* ScalarOrVector(uint32_t width, T value) {
        if (width == 1) {
            return b.Expr(value);
        }
        return b.Call(b.ty.vec<T>(width), value);
    }

    template <typename To>
    const ast::Expression* CastScalarOrVector(uint32_t width, const ast::Expression* e) {
        if (width == 1) {
            return b.Call(b.ty.Of<To>(), e);
        }
        return b.Call(b.ty.vec<To>(width), e);
    }

    /// Examines the call expression @p expr, applying any necessary polyfill transforms
    void Call(const ast::CallExpression* expr) {
        auto* call = src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        if (!call || call->Stage() == sem::EvaluationStage::kConstant ||
            call->Stage() == sem::EvaluationStage::kNotEvaluated) {
            return;  // Don't polyfill @const expressions
        }
        Symbol fn = Switch(
            call->Target(),  //
            [&](const sem::Builtin* builtin) {
                switch (builtin->Type()) {
                    case builtin::Function::kAcosh:
                        if (cfg.builtins.acosh != Level::kNone) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return acosh(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kAsinh:
                        if (cfg.builtins.asinh) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return asinh(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kAtanh:
                        if (cfg.builtins.atanh != Level::kNone) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return atanh(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kClamp:
                        if (cfg.builtins.clamp_int) {
                            auto& sig = builtin->Signature();
                            if (sig.parameters[0]->Type()->is_integer_scalar_or_vector()) {
                                return builtin_polyfills.GetOrCreate(
                                    builtin, [&] { return clampInteger(builtin->ReturnType()); });
                            }
                        }
                        return Symbol{};

                    case builtin::Function::kCountLeadingZeros:
                        if (cfg.builtins.count_leading_zeros) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return countLeadingZeros(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kCountTrailingZeros:
                        if (cfg.builtins.count_trailing_zeros) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return countTrailingZeros(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kExtractBits:
                        if (cfg.builtins.extract_bits != Level::kNone) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return extractBits(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kFirstLeadingBit:
                        if (cfg.builtins.first_leading_bit) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return firstLeadingBit(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kFirstTrailingBit:
                        if (cfg.builtins.first_trailing_bit) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return firstTrailingBit(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kInsertBits:
                        if (cfg.builtins.insert_bits != Level::kNone) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return insertBits(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kReflect:
                        // Only polyfill for vec2<f32>. See https://crbug.com/tint/1798 for
                        // more details.
                        if (cfg.builtins.reflect_vec2_f32) {
                            auto& sig = builtin->Signature();
                            auto* vec = sig.return_type->As<type::Vector>();
                            if (vec && vec->Width() == 2 && vec->type()->Is<type::F32>()) {
                                return builtin_polyfills.GetOrCreate(
                                    builtin, [&] { return reflect(builtin->ReturnType()); });
                            }
                        }
                        return Symbol{};

                    case builtin::Function::kSaturate:
                        if (cfg.builtins.saturate) {
                            return builtin_polyfills.GetOrCreate(
                                builtin, [&] { return saturate(builtin->ReturnType()); });
                        }
                        return Symbol{};

                    case builtin::Function::kSign:
                        if (cfg.builtins.sign_int) {
                            auto* ty = builtin->ReturnType();
                            if (ty->is_signed_integer_scalar_or_vector()) {
                                return builtin_polyfills.GetOrCreate(builtin,
                                                                     [&] { return sign_int(ty); });
                            }
                        }
                        return Symbol{};

                    case builtin::Function::kTextureSampleBaseClampToEdge:
                        if (cfg.builtins.texture_sample_base_clamp_to_edge_2d_f32) {
                            auto& sig = builtin->Signature();
                            auto* tex = sig.Parameter(sem::ParameterUsage::kTexture);
                            if (auto* stex = tex->Type()->As<type::SampledTexture>()) {
                                if (stex->type()->Is<type::F32>()) {
                                    return builtin_polyfills.GetOrCreate(builtin, [&] {
                                        return textureSampleBaseClampToEdge_2d_f32();
                                    });
                                }
                            }
                        }
                        return Symbol{};

                    case builtin::Function::kTextureStore:
                        if (cfg.builtins.bgra8unorm) {
                            auto& sig = builtin->Signature();
                            auto* tex = sig.Parameter(sem::ParameterUsage::kTexture);
                            if (auto* stex = tex->Type()->As<type::StorageTexture>()) {
                                if (stex->texel_format() == builtin::TexelFormat::kBgra8Unorm) {
                                    size_t value_idx = static_cast<size_t>(
                                        sig.IndexOf(sem::ParameterUsage::kValue));
                                    ctx.Replace(expr, [this, expr, value_idx] {
                                        utils::Vector<const ast::Expression*, 3> args;
                                        for (auto* arg : expr->args) {
                                            arg = ctx.Clone(arg);
                                            if (args.Length() == value_idx) {  // value
                                                arg = ctx.dst->MemberAccessor(arg, "bgra");
                                            }
                                            args.Push(arg);
                                        }
                                        return ctx.dst->Call(
                                            utils::ToString(builtin::Function::kTextureStore),
                                            std::move(args));
                                    });
                                    made_changes = true;
                                }
                            }
                        }
                        return Symbol{};

                    case builtin::Function::kQuantizeToF16:
                        if (cfg.builtins.quantize_to_vec_f16) {
                            if (auto* vec = builtin->ReturnType()->As<type::Vector>()) {
                                return builtin_polyfills.GetOrCreate(
                                    builtin, [&] { return quantizeToF16(vec); });
                            }
                        }
                        return Symbol{};

                    case builtin::Function::kWorkgroupUniformLoad:
                        if (cfg.builtins.workgroup_uniform_load) {
                            return builtin_polyfills.GetOrCreate(builtin, [&] {
                                return workgroupUniformLoad(builtin->ReturnType());
                            });
                        }
                        return Symbol{};

                    default:
                        return Symbol{};
                }
            },
            [&](const sem::ValueConversion* conv) {
                if (cfg.builtins.conv_f32_to_iu32) {
                    auto* src_ty = conv->Source();
                    if (tint::Is<type::F32>(type::Type::ElementOf(src_ty))) {
                        auto* dst_ty = conv->Target();
                        if (tint::utils::IsAnyOf<type::I32, type::U32>(
                                type::Type::ElementOf(dst_ty))) {
                            return f32_conv_polyfills.GetOrCreate(dst_ty, [&] {  //
                                return ConvF32ToIU32(src_ty, dst_ty);
                            });
                        }
                    }
                }
                return Symbol{};
            });

        if (fn.IsValid()) {
            ctx.Replace(call->Declaration(),
                        [this, fn, expr] { return ctx.dst->Call(fn, ctx.Clone(expr->args)); });
            made_changes = true;
        }
    }
};

BuiltinPolyfill::BuiltinPolyfill() = default;

BuiltinPolyfill::~BuiltinPolyfill() = default;

Transform::ApplyResult BuiltinPolyfill::Apply(const Program* src,
                                              const DataMap& data,
                                              DataMap&) const {
    auto* cfg = data.Get<Config>();
    if (!cfg) {
        return SkipTransform;
    }
    return State{src, *cfg}.Run();
}

BuiltinPolyfill::Config::Config(const Builtins& b) : builtins(b) {}
BuiltinPolyfill::Config::Config(const Config&) = default;
BuiltinPolyfill::Config::~Config() = default;

}  // namespace tint::transform
