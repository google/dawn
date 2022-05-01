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

#include "src/tint/program_builder.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/call.h"
#include "src/tint/utils/map.h"

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
            return ScalarOrVector(width, value);
        };
        b.Func(
            name, {b.Param("v", T(ty))}, T(ty),
            {
                // var x = U(v);
                b.Decl(b.Var("x", nullptr, b.Construct(U(), b.Expr("v")))),
                // let b16 = select(0, 16, x <= 0x0000ffff);
                b.Decl(b.Let("b16", nullptr,
                             b.Call("select", V(0), V(16), b.LessThanEqual("x", V(0x0000ffff))))),
                // x = x << b16;
                b.Assign("x", b.Shl("x", "b16")),
                // let b8  = select(0, 8,  x <= 0x00ffffff);
                b.Decl(b.Let("b8", nullptr,
                             b.Call("select", V(0), V(8), b.LessThanEqual("x", V(0x00ffffff))))),
                // x = x << b8;
                b.Assign("x", b.Shl("x", "b8")),
                // let b4  = select(0, 4,  x <= 0x0fffffff);
                b.Decl(b.Let("b4", nullptr,
                             b.Call("select", V(0), V(4), b.LessThanEqual("x", V(0x0fffffff))))),
                // x = x << b4;
                b.Assign("x", b.Shl("x", "b4")),
                // let b2  = select(0, 2,  x <= 0x3fffffff);
                b.Decl(b.Let("b2", nullptr,
                             b.Call("select", V(0), V(2), b.LessThanEqual("x", V(0x3fffffff))))),
                // x = x << b2;
                b.Assign("x", b.Shl("x", "b2")),
                // let b1  = select(0, 1,  x <= 0x7fffffff);
                b.Decl(b.Let("b1", nullptr,
                             b.Call("select", V(0), V(1), b.LessThanEqual("x", V(0x7fffffff))))),
                // let is_zero  = select(0, 1, x == 0);
                b.Decl(b.Let("is_zero", nullptr, b.Call("select", V(0), V(1), b.Equal("x", V(0))))),
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
            return ScalarOrVector(width, value);
        };
        auto B = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return b.Construct<bool>(value);
            }
            return b.Construct(b.ty.vec<bool>(width), value);
        };
        b.Func(
            name, {b.Param("v", T(ty))}, T(ty),
            {
                // var x = U(v);
                b.Decl(b.Var("x", nullptr, b.Construct(U(), b.Expr("v")))),
                // let b16 = select(16, 0, bool(x & 0x0000ffff));
                b.Decl(b.Let("b16", nullptr,
                             b.Call("select", V(16), V(0), B(b.And("x", V(0x0000ffff)))))),
                // x = x >> b16;
                b.Assign("x", b.Shr("x", "b16")),
                // let b8  = select(8,  0, bool(x & 0x000000ff));
                b.Decl(b.Let("b8", nullptr,
                             b.Call("select", V(8), V(0), B(b.And("x", V(0x000000ff)))))),
                // x = x >> b8;
                b.Assign("x", b.Shr("x", "b8")),
                // let b4  = select(4,  0, bool(x & 0x0000000f));
                b.Decl(b.Let("b4", nullptr,
                             b.Call("select", V(4), V(0), B(b.And("x", V(0x0000000f)))))),
                // x = x >> b4;
                b.Assign("x", b.Shr("x", "b4")),
                // let b2  = select(2,  0, bool(x & 0x00000003));
                b.Decl(b.Let("b2", nullptr,
                             b.Call("select", V(2), V(0), B(b.And("x", V(0x00000003)))))),
                // x = x >> b2;
                b.Assign("x", b.Shr("x", "b2")),
                // let b1  = select(1,  0, bool(x & 0x00000001));
                b.Decl(b.Let("b1", nullptr,
                             b.Call("select", V(1), V(0), B(b.And("x", V(0x00000001)))))),
                // let is_zero  = select(0, 1, x == 0);
                b.Decl(b.Let("is_zero", nullptr, b.Call("select", V(0), V(1), b.Equal("x", V(0))))),
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

        ast::StatementList body = {
            b.Decl(b.Let("s", nullptr, b.Call("min", "offset", W))),
            b.Decl(b.Let("e", nullptr, b.Call("min", W, b.Add("s", "count")))),
        };

        switch (polyfill.extract_bits) {
            case Level::kFull:
                body.emplace_back(b.Decl(b.Let("shl", nullptr, b.Sub(W, "e"))));
                body.emplace_back(b.Decl(b.Let("shr", nullptr, b.Add("shl", "s"))));
                body.emplace_back(
                    b.Return(b.Shr(b.Shl("v", vecN_u32(b.Expr("shl"))), vecN_u32(b.Expr("shr")))));
                break;
            case Level::kClampParameters:
                body.emplace_back(b.Return(b.Call("extractBits", "v", "s", b.Sub("e", "s"))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(polyfill.extract_bits);
                return {};
        }

        b.Func(name,
               {
                   b.Param("v", T(ty)),
                   b.Param("offset", b.ty.u32()),
                   b.Param("count", b.ty.u32()),
               },
               T(ty), body);

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
            return ScalarOrVector(width, value);
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
                       b.LessThan("v", ScalarOrVector(width, 0)));
        }

        b.Func(
            name, {b.Param("v", T(ty))}, T(ty),
            {
                // var x = v;                          (unsigned)
                // var x = select(U(v), ~U(v), v < 0); (signed)
                b.Decl(b.Var("x", nullptr, x)),
                // let b16 = select(0, 16, bool(x & 0xffff0000));
                b.Decl(b.Let("b16", nullptr,
                             b.Call("select", V(0), V(16), B(b.And("x", V(0xffff0000)))))),
                // x = x >> b16;
                b.Assign("x", b.Shr("x", "b16")),
                // let b8  = select(0, 8,  bool(x & 0x0000ff00));
                b.Decl(b.Let("b8", nullptr,
                             b.Call("select", V(0), V(8), B(b.And("x", V(0x0000ff00)))))),
                // x = x >> b8;
                b.Assign("x", b.Shr("x", "b8")),
                // let b4  = select(0, 4,  bool(x & 0x000000f0));
                b.Decl(b.Let("b4", nullptr,
                             b.Call("select", V(0), V(4), B(b.And("x", V(0x000000f0)))))),
                // x = x >> b4;
                b.Assign("x", b.Shr("x", "b4")),
                // let b2  = select(0, 2,  bool(x & 0x0000000c));
                b.Decl(b.Let("b2", nullptr,
                             b.Call("select", V(0), V(2), B(b.And("x", V(0x0000000c)))))),
                // x = x >> b2;
                b.Assign("x", b.Shr("x", "b2")),
                // let b1  = select(0, 1,  bool(x & 0x00000002));
                b.Decl(b.Let("b1", nullptr,
                             b.Call("select", V(0), V(1), B(b.And("x", V(0x00000002)))))),
                // let is_zero  = select(0, 0xffffffff, x == 0);
                b.Decl(b.Let("is_zero", nullptr,
                             b.Call("select", V(0), V(0xffffffff), b.Equal("x", V(0))))),
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
            return ScalarOrVector(width, value);
        };
        auto B = [&](const ast::Expression* value) -> const ast::Expression* {
            if (width == 1) {
                return b.Construct<bool>(value);
            }
            return b.Construct(b.ty.vec<bool>(width), value);
        };
        b.Func(
            name, {b.Param("v", T(ty))}, T(ty),
            {
                // var x = U(v);
                b.Decl(b.Var("x", nullptr, b.Construct(U(), b.Expr("v")))),
                // let b16 = select(16, 0, bool(x & 0x0000ffff));
                b.Decl(b.Let("b16", nullptr,
                             b.Call("select", V(16), V(0), B(b.And("x", V(0x0000ffff)))))),
                // x = x >> b16;
                b.Assign("x", b.Shr("x", "b16")),
                // let b8  = select(8,  0, bool(x & 0x000000ff));
                b.Decl(b.Let("b8", nullptr,
                             b.Call("select", V(8), V(0), B(b.And("x", V(0x000000ff)))))),
                // x = x >> b8;
                b.Assign("x", b.Shr("x", "b8")),
                // let b4  = select(4,  0, bool(x & 0x0000000f));
                b.Decl(b.Let("b4", nullptr,
                             b.Call("select", V(4), V(0), B(b.And("x", V(0x0000000f)))))),
                // x = x >> b4;
                b.Assign("x", b.Shr("x", "b4")),
                // let b2  = select(2,  0, bool(x & 0x00000003));
                b.Decl(b.Let("b2", nullptr,
                             b.Call("select", V(2), V(0), B(b.And("x", V(0x00000003)))))),
                // x = x >> b2;
                b.Assign("x", b.Shr("x", "b2")),
                // let b1  = select(1,  0, bool(x & 0x00000001));
                b.Decl(b.Let("b1", nullptr,
                             b.Call("select", V(1), V(0), B(b.And("x", V(0x00000001)))))),
                // let is_zero  = select(0, 0xffffffff, x == 0);
                b.Decl(b.Let("is_zero", nullptr,
                             b.Call("select", V(0), V(0xffffffff), b.Equal("x", V(0))))),
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

        ast::StatementList body = {
            b.Decl(b.Let("s", nullptr, b.Call("min", "offset", W))),
            b.Decl(b.Let("e", nullptr, b.Call("min", W, b.Add("s", "count")))),
        };

        switch (polyfill.insert_bits) {
            case Level::kFull:
                // let mask = ((1 << s) - 1) ^ ((1 << e) - 1)
                body.emplace_back(b.Decl(b.Let(
                    "mask", nullptr, b.Xor(b.Sub(b.Shl(1u, "s"), 1u), b.Sub(b.Shl(1u, "e"), 1u)))));
                // return ((n << s) & mask) | (v & ~mask)
                body.emplace_back(b.Return(b.Or(b.And(b.Shl("n", U("s")), V("mask")),
                                                b.And("v", V(b.Complement("mask"))))));
                break;
            case Level::kClampParameters:
                body.emplace_back(b.Return(b.Call("insertBits", "v", "n", "s", b.Sub("e", "s"))));
                break;
            default:
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled polyfill level: " << static_cast<int>(polyfill.insert_bits);
                return {};
        }

        b.Func(name,
               {
                   b.Param("v", T(ty)),
                   b.Param("n", T(ty)),
                   b.Param("offset", b.ty.u32()),
                   b.Param("count", b.ty.u32()),
               },
               T(ty), body);

        return name;
    }

  private:
    /// Aliases
    using u32 = ProgramBuilder::u32;
    using i32 = ProgramBuilder::i32;

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
