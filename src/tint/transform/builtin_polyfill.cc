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

#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/call.h"
#include "src/tint/utils/map.h"

using namespace tint::number_suffixes;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::transform::BuiltinPolyfill);
TINT_INSTANTIATE_TYPEINFO(tint::transform::BuiltinPolyfill::Config);

namespace tint::transform {

/// The PIMPL state for the BuiltinPolyfill transform
struct BuiltinPolyfill::State {
    /// Constructor
    /// @param c the CloneContext
    /// @param p the builtins to polyfill
    State(CloneContext& c, Builtins p) : ctx(c), polyfill(p) {}

    /// The clone context
    CloneContext& ctx;
    /// The builtins to polyfill
    Builtins polyfill;
    /// The destination program builder
    ProgramBuilder& b = *ctx.dst;
    /// The source clone context
    const sem::Info& sem = ctx.src->Sem();

    /// Builds the polyfill function for the `acosh` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol acosh(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_acosh");
        uint32_t width = WidthOf(ty);

        auto V = [&](AFloat value) -> const ast::Expression* {
            const ast::Expression* expr = b.Expr(value);
            if (width == 1) {
                return expr;
            }
            return b.Construct(T(ty), expr);
        };

        utils::Vector<const ast::Statement*, 4> body;
        switch (polyfill.acosh) {
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
                    << "unhandled polyfill level: " << static_cast<int>(polyfill.acosh);
                return {};
        }

        b.Func(name, utils::Vector{b.Param("x", T(ty))}, T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `asinh` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol asinh(const sem::Type* ty) {
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
    Symbol atanh(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_atanh");
        uint32_t width = WidthOf(ty);

        auto V = [&](AFloat value) -> const ast::Expression* {
            const ast::Expression* expr = b.Expr(value);
            if (width == 1) {
                return expr;
            }
            return b.Construct(T(ty), expr);
        };

        utils::Vector<const ast::Statement*, 1> body;
        switch (polyfill.atanh) {
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
                    << "unhandled polyfill level: " << static_cast<int>(polyfill.acosh);
                return {};
        }

        b.Func(name, utils::Vector{b.Param("x", T(ty))}, T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `countLeadingZeros` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol countLeadingZeros(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_count_leading_zeros");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() -> const ast::Type* {
            if (width == 1) {
                return b.ty.u32();
            }
            return b.ty.vec<u32>(width);
        };
        auto V = [&](uint32_t value) -> const ast::Expression* {
            return ScalarOrVector(width, u32(value));
        };
        b.Func(name,
               utils::Vector{
                   b.Param("v", T(ty)),
               },
               T(ty),
               utils::Vector{
                   // var x = U(v);
                   b.Decl(b.Var("x", b.Construct(U(), b.Expr("v")))),
                   // let b16 = select(0, 16, x <= 0x0000ffff);
                   b.Decl(b.Let(
                       "b16", b.Call("select", V(0), V(16), b.LessThanEqual("x", V(0x0000ffff))))),
                   // x = x << b16;
                   b.Assign("x", b.Shl("x", "b16")),
                   // let b8  = select(0, 8,  x <= 0x00ffffff);
                   b.Decl(b.Let("b8",
                                b.Call("select", V(0), V(8), b.LessThanEqual("x", V(0x00ffffff))))),
                   // x = x << b8;
                   b.Assign("x", b.Shl("x", "b8")),
                   // let b4  = select(0, 4,  x <= 0x0fffffff);
                   b.Decl(b.Let("b4",
                                b.Call("select", V(0), V(4), b.LessThanEqual("x", V(0x0fffffff))))),
                   // x = x << b4;
                   b.Assign("x", b.Shl("x", "b4")),
                   // let b2  = select(0, 2,  x <= 0x3fffffff);
                   b.Decl(b.Let("b2",
                                b.Call("select", V(0), V(2), b.LessThanEqual("x", V(0x3fffffff))))),
                   // x = x << b2;
                   b.Assign("x", b.Shl("x", "b2")),
                   // let b1  = select(0, 1,  x <= 0x7fffffff);
                   b.Decl(b.Let("b1",
                                b.Call("select", V(0), V(1), b.LessThanEqual("x", V(0x7fffffff))))),
                   // let is_zero  = select(0, 1, x == 0);
                   b.Decl(b.Let("is_zero", b.Call("select", V(0), V(1), b.Equal("x", V(0))))),
                   // return R((b16 | b8 | b4 | b2 | b1) + zero);
                   b.Return(b.Construct(
                       T(ty),
                       b.Add(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"), "is_zero"))),
               });
        return name;
    }

    /// Builds the polyfill function for the `countTrailingZeros` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol countTrailingZeros(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_count_trailing_zeros");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() -> const ast::Type* {
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
                return b.Construct<bool>(value);
            }
            return b.Construct(b.ty.vec<bool>(width), value);
        };
        b.Func(
            name,
            utils::Vector{
                b.Param("v", T(ty)),
            },
            T(ty),
            utils::Vector{
                // var x = U(v);
                b.Decl(b.Var("x", b.Construct(U(), b.Expr("v")))),
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
                b.Return(b.Construct(
                    T(ty),
                    b.Add(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"), "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `extractBits` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol extractBits(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_extract_bits");
        uint32_t width = WidthOf(ty);

        constexpr uint32_t W = 32u;  // 32-bit

        auto vecN_u32 = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return value;
            }
            return b.Construct(b.ty.vec<u32>(width), value);
        };

        utils::Vector<const ast::Statement*, 8> body{
            b.Decl(b.Let("s", b.Call("min", "offset", u32(W)))),
            b.Decl(b.Let("e", b.Call("min", u32(W), b.Add("s", "count")))),
        };

        switch (polyfill.extract_bits) {
            case Level::kFull:
                body.Push(b.Decl(b.Let("shl", b.Sub(u32(W), "e"))));
                body.Push(b.Decl(b.Let("shr", b.Add("shl", "s"))));
                body.Push(
                    b.Return(b.Shr(b.Shl("v", vecN_u32(b.Expr("shl"))), vecN_u32(b.Expr("shr")))));
                break;
            case Level::kClampParameters:
                body.Push(b.Return(b.Call("extractBits", "v", "s", b.Sub("e", "s"))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(polyfill.extract_bits);
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
    Symbol firstLeadingBit(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_first_leading_bit");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() -> const ast::Type* {
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
                return b.Construct<bool>(value);
            }
            return b.Construct(b.ty.vec<bool>(width), value);
        };

        const ast::Expression* x = nullptr;
        if (ty->is_unsigned_scalar_or_vector()) {
            x = b.Expr("v");
        } else {
            // If ty is signed, then the value is inverted if the sign is negative
            x = b.Call("select",                             //
                       b.Construct(U(), "v"),                //
                       b.Construct(U(), b.Complement("v")),  //
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
                b.Return(b.Construct(
                    T(ty), b.Or(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"), "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `firstTrailingBit` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol firstTrailingBit(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_first_trailing_bit");
        uint32_t width = WidthOf(ty);

        // Returns either u32 or vecN<u32>
        auto U = [&]() -> const ast::Type* {
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
                return b.Construct<bool>(value);
            }
            return b.Construct(b.ty.vec<bool>(width), value);
        };
        b.Func(
            name,
            utils::Vector{
                b.Param("v", T(ty)),
            },
            T(ty),
            utils::Vector{
                // var x = U(v);
                b.Decl(b.Var("x", b.Construct(U(), b.Expr("v")))),
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
                b.Return(b.Construct(
                    T(ty), b.Or(b.Or(b.Or(b.Or(b.Or("b16", "b8"), "b4"), "b2"), "b1"), "is_zero"))),
            });
        return name;
    }

    /// Builds the polyfill function for the `insertBits` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol insertBits(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_insert_bits");
        uint32_t width = WidthOf(ty);

        constexpr uint32_t W = 32u;  // 32-bit

        auto V = [&](auto value) -> const ast::Expression* {
            const ast::Expression* expr = b.Expr(value);
            if (!ty->is_unsigned_scalar_or_vector()) {
                expr = b.Construct<i32>(expr);
            }
            if (ty->Is<sem::Vector>()) {
                expr = b.Construct(T(ty), expr);
            }
            return expr;
        };
        auto U = [&](auto value) -> const ast::Expression* {
            if (width == 1) {
                return b.Expr(value);
            }
            return b.vec(b.ty.u32(), width, value);
        };

        utils::Vector<const ast::Statement*, 8> body = {
            b.Decl(b.Let("s", b.Call("min", "offset", u32(W)))),
            b.Decl(b.Let("e", b.Call("min", u32(W), b.Add("s", "count")))),
        };

        switch (polyfill.insert_bits) {
            case Level::kFull:
                // let mask = ((1 << s) - 1) ^ ((1 << e) - 1)
                body.Push(b.Decl(b.Let(
                    "mask", b.Xor(b.Sub(b.Shl(1_u, "s"), 1_u), b.Sub(b.Shl(1_u, "e"), 1_u)))));
                // return ((n << s) & mask) | (v & ~mask)
                body.Push(b.Return(b.Or(b.And(b.Shl("n", U("s")), V("mask")),
                                        b.And("v", V(b.Complement("mask"))))));
                break;
            case Level::kClampParameters:
                body.Push(b.Return(b.Call("insertBits", "v", "n", "s", b.Sub("e", "s"))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(polyfill.insert_bits);
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

    /// Builds the polyfill function for the `saturate` builtin
    /// @param ty the parameter and return type for the function
    /// @return the polyfill function name
    Symbol saturate(const sem::Type* ty) {
        auto name = b.Symbols().New("tint_saturate");
        auto body = utils::Vector{
            b.Return(b.Call("clamp", "v", b.Construct(T(ty), 0_a), b.Construct(T(ty), 1_a))),
        };
        b.Func(name,
               utils::Vector{
                   b.Param("v", T(ty)),
               },
               T(ty), body);

        return name;
    }

    /// Builds the polyfill function for the `textureSampleBaseClampToEdge` builtin, when the
    /// texture type is texture_2d<f32>.
    /// @return the polyfill function name
    Symbol textureSampleBaseClampToEdge_2d_f32() {
        auto name = b.Symbols().New("tint_textureSampleBaseClampToEdge");
        auto body = utils::Vector{
            b.Decl(b.Let("dims",
                         b.Construct(b.ty.vec2<f32>(), b.Call("textureDimensions", "t", 0_a)))),
            b.Decl(b.Let("half_texel", b.Div(b.vec2<f32>(0.5_a), "dims"))),
            b.Decl(
                b.Let("clamped", b.Call("clamp", "coord", "half_texel", b.Sub(1_a, "half_texel")))),
            b.Return(b.Call("textureSampleLevel", "t", "s", "clamped", 0_a)),
        };
        b.Func(name,
               utils::Vector{
                   b.Param("t", b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32())),
                   b.Param("s", b.ty.sampler(ast::SamplerKind::kSampler)),
                   b.Param("coord", b.ty.vec2<f32>()),
               },
               b.ty.vec4<f32>(), body);
        return name;
    }

  private:
    /// @returns the AST type for the given sem type
    const ast::Type* T(const sem::Type* ty) const { return CreateASTTypeFor(ctx, ty); }

    /// @returns 1 if `ty` is not a vector, otherwise the vector width
    uint32_t WidthOf(const sem::Type* ty) const {
        if (auto* v = ty->As<sem::Vector>()) {
            return v->Width();
        }
        return 1;
    }

    /// @returns a scalar or vector with the given width, with each element with
    /// the given value.
    template <typename T>
    const ast::Expression* ScalarOrVector(uint32_t width, T value) const {
        if (width == 1) {
            return b.Expr(value);
        }
        return b.Construct(b.ty.vec<T>(width), value);
    }
};

BuiltinPolyfill::BuiltinPolyfill() = default;

BuiltinPolyfill::~BuiltinPolyfill() = default;

bool BuiltinPolyfill::ShouldRun(const Program* program, const DataMap& data) const {
    if (auto* cfg = data.Get<Config>()) {
        auto builtins = cfg->builtins;
        auto& sem = program->Sem();
        for (auto* node : program->ASTNodes().Objects()) {
            if (auto* call = sem.Get<sem::Call>(node)) {
                if (auto* builtin = call->Target()->As<sem::Builtin>()) {
                    switch (builtin->Type()) {
                        case sem::BuiltinType::kAcosh:
                            if (builtins.acosh != Level::kNone) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kAsinh:
                            if (builtins.asinh) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kAtanh:
                            if (builtins.atanh != Level::kNone) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kCountLeadingZeros:
                            if (builtins.count_leading_zeros) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kCountTrailingZeros:
                            if (builtins.count_trailing_zeros) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kExtractBits:
                            if (builtins.extract_bits != Level::kNone) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kFirstLeadingBit:
                            if (builtins.first_leading_bit) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kFirstTrailingBit:
                            if (builtins.first_trailing_bit) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kInsertBits:
                            if (builtins.insert_bits != Level::kNone) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kSaturate:
                            if (builtins.saturate) {
                                return true;
                            }
                            break;
                        case sem::BuiltinType::kTextureSampleBaseClampToEdge:
                            if (builtins.texture_sample_base_clamp_to_edge_2d_f32) {
                                auto& sig = builtin->Signature();
                                auto* tex = sig.Parameter(sem::ParameterUsage::kTexture);
                                if (auto* stex = tex->Type()->As<sem::SampledTexture>()) {
                                    return stex->type()->Is<sem::F32>();
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    return false;
}

void BuiltinPolyfill::Run(CloneContext& ctx, const DataMap& data, DataMap&) const {
    auto* cfg = data.Get<Config>();
    if (!cfg) {
        ctx.Clone();
        return;
    }

    std::unordered_map<const sem::Builtin*, Symbol> polyfills;

    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::CallExpression* {
        auto builtins = cfg->builtins;
        State s{ctx, builtins};
        if (auto* call = s.sem.Get<sem::Call>(expr)) {
            if (auto* builtin = call->Target()->As<sem::Builtin>()) {
                Symbol polyfill;
                switch (builtin->Type()) {
                    case sem::BuiltinType::kAcosh:
                        if (builtins.acosh != Level::kNone) {
                            polyfill = utils::GetOrCreate(
                                polyfills, builtin, [&] { return s.acosh(builtin->ReturnType()); });
                        }
                        break;
                    case sem::BuiltinType::kAsinh:
                        if (builtins.asinh) {
                            polyfill = utils::GetOrCreate(
                                polyfills, builtin, [&] { return s.asinh(builtin->ReturnType()); });
                        }
                        break;
                    case sem::BuiltinType::kAtanh:
                        if (builtins.atanh != Level::kNone) {
                            polyfill = utils::GetOrCreate(
                                polyfills, builtin, [&] { return s.atanh(builtin->ReturnType()); });
                        }
                        break;
                    case sem::BuiltinType::kCountLeadingZeros:
                        if (builtins.count_leading_zeros) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.countLeadingZeros(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kCountTrailingZeros:
                        if (builtins.count_trailing_zeros) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.countTrailingZeros(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kExtractBits:
                        if (builtins.extract_bits != Level::kNone) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.extractBits(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kFirstLeadingBit:
                        if (builtins.first_leading_bit) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.firstLeadingBit(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kFirstTrailingBit:
                        if (builtins.first_trailing_bit) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.firstTrailingBit(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kInsertBits:
                        if (builtins.insert_bits != Level::kNone) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.insertBits(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kSaturate:
                        if (builtins.saturate) {
                            polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                return s.saturate(builtin->ReturnType());
                            });
                        }
                        break;
                    case sem::BuiltinType::kTextureSampleBaseClampToEdge:
                        if (builtins.texture_sample_base_clamp_to_edge_2d_f32) {
                            auto& sig = builtin->Signature();
                            auto* tex = sig.Parameter(sem::ParameterUsage::kTexture);
                            if (auto* stex = tex->Type()->As<sem::SampledTexture>()) {
                                if (stex->type()->Is<sem::F32>()) {
                                    polyfill = utils::GetOrCreate(polyfills, builtin, [&] {
                                        return s.textureSampleBaseClampToEdge_2d_f32();
                                    });
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
                if (polyfill.IsValid()) {
                    return s.b.Call(polyfill, ctx.Clone(call->Declaration()->args));
                }
            }
        }
        return nullptr;
    });

    ctx.Clone();
}

BuiltinPolyfill::Config::Config(const Builtins& b) : builtins(b) {}
BuiltinPolyfill::Config::Config(const Config&) = default;
BuiltinPolyfill::Config::~Config() = default;

}  // namespace tint::transform
