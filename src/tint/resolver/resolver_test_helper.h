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

#ifndef SRC_TINT_RESOLVER_RESOLVER_TEST_HELPER_H_
#define SRC_TINT_RESOLVER_RESOLVER_TEST_HELPER_H_

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

#include "gtest/gtest.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/utils/traits.h"
#include "src/tint/utils/vector.h"

namespace tint::resolver {

/// Helper class for testing
class TestHelper : public ProgramBuilder {
  public:
    /// Constructor
    TestHelper();

    /// Destructor
    ~TestHelper() override;

    /// @return a pointer to the Resolver
    Resolver* r() const { return resolver_.get(); }

    /// @return a pointer to the validator
    const Validator* v() const { return resolver_->GetValidatorForTesting(); }

    /// Returns the statement that holds the given expression.
    /// @param expr the ast::Expression
    /// @return the ast::Statement of the ast::Expression, or nullptr if the
    /// expression is not owned by a statement.
    const ast::Statement* StmtOf(const ast::Expression* expr) {
        auto* sem_stmt = Sem().Get(expr)->Stmt();
        return sem_stmt ? sem_stmt->Declaration() : nullptr;
    }

    /// Returns the BlockStatement that holds the given statement.
    /// @param stmt the ast::Statement
    /// @return the ast::BlockStatement that holds the ast::Statement, or nullptr
    /// if the statement is not owned by a BlockStatement.
    const ast::BlockStatement* BlockOf(const ast::Statement* stmt) {
        auto* sem_stmt = Sem().Get(stmt);
        return sem_stmt ? sem_stmt->Block()->Declaration() : nullptr;
    }

    /// Returns the BlockStatement that holds the given expression.
    /// @param expr the ast::Expression
    /// @return the ast::Statement of the ast::Expression, or nullptr if the
    /// expression is not indirectly owned by a BlockStatement.
    const ast::BlockStatement* BlockOf(const ast::Expression* expr) {
        auto* sem_stmt = Sem().Get(expr)->Stmt();
        return sem_stmt ? sem_stmt->Block()->Declaration() : nullptr;
    }

    /// Returns the semantic variable for the given identifier expression.
    /// @param expr the identifier expression
    /// @return the resolved sem::Variable of the identifier, or nullptr if
    /// the expression did not resolve to a variable.
    const sem::Variable* VarOf(const ast::Expression* expr) {
        if (auto* sem = Sem().GetVal(expr)) {
            if (auto* var_user = As<sem::VariableUser>(sem->UnwrapLoad())) {
                return var_user->Variable();
            }
        }
        return nullptr;
    }

    /// Checks that all the users of the given variable are as expected
    /// @param var the variable to check
    /// @param expected_users the expected users of the variable
    /// @return true if all users are as expected
    bool CheckVarUsers(const ast::Variable* var,
                       utils::VectorRef<const ast::Expression*> expected_users) {
        auto var_users = Sem().Get(var)->Users();
        if (var_users.Length() != expected_users.Length()) {
            return false;
        }
        for (size_t i = 0; i < var_users.Length(); i++) {
            if (var_users[i]->Declaration() != expected_users[i]) {
                return false;
            }
        }
        return true;
    }

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(ast::Type type) { return type->identifier->symbol.Name(); }

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const type::Type* type) { return type->FriendlyName(); }

  private:
    std::unique_ptr<Resolver> resolver_;
};

class ResolverTest : public TestHelper, public testing::Test {};

template <typename T>
class ResolverTestWithParam : public TestHelper, public testing::TestWithParam<T> {};

namespace builder {

template <uint32_t N, typename T>
struct vec {};

template <typename T>
using vec2 = vec<2, T>;

template <typename T>
using vec3 = vec<3, T>;

template <typename T>
using vec4 = vec<4, T>;

template <uint32_t N, uint32_t M, typename T>
struct mat {};

template <typename T>
using mat2x2 = mat<2, 2, T>;

template <typename T>
using mat2x3 = mat<2, 3, T>;

template <typename T>
using mat2x4 = mat<2, 4, T>;

template <typename T>
using mat3x2 = mat<3, 2, T>;

template <typename T>
using mat3x3 = mat<3, 3, T>;

template <typename T>
using mat3x4 = mat<3, 4, T>;

template <typename T>
using mat4x2 = mat<4, 2, T>;

template <typename T>
using mat4x3 = mat<4, 3, T>;

template <typename T>
using mat4x4 = mat<4, 4, T>;

template <uint32_t N, typename T>
struct array {};

template <typename TO, int ID = 0>
struct alias {};

template <typename TO>
using alias1 = alias<TO, 1>;

template <typename TO>
using alias2 = alias<TO, 2>;

template <typename TO>
using alias3 = alias<TO, 3>;

template <typename TO>
struct ptr {};

/// A scalar value
using Scalar = std::variant<i32, u32, f32, f16, AInt, AFloat, bool>;

/// Returns current variant value in `s` cast to type `T`
template <typename T>
T As(const Scalar& s) {
    return std::visit([](auto&& v) { return static_cast<T>(v); }, s);
}

using ast_type_func_ptr = ast::Type (*)(ProgramBuilder& b);
using ast_expr_func_ptr = const ast::Expression* (*)(ProgramBuilder& b,
                                                     utils::VectorRef<Scalar> args);
using ast_expr_from_double_func_ptr = const ast::Expression* (*)(ProgramBuilder& b, double v);
using sem_type_func_ptr = const type::Type* (*)(ProgramBuilder& b);
using type_name_func_ptr = std::string (*)();

struct UnspecializedElementType {};

/// Base template for DataType, specialized below.
template <typename T>
struct DataType {
    /// The element type
    using ElementType = UnspecializedElementType;
};

/// Helper that represents no-type. Returns nullptr for all static methods.
template <>
struct DataType<void> {
    /// The element type
    using ElementType = void;

    /// @return nullptr
    static inline ast::Type AST(ProgramBuilder&) { return {}; }
    /// @return nullptr
    static inline const type::Type* Sem(ProgramBuilder&) { return nullptr; }
};

/// Helper for building bool types and expressions
template <>
struct DataType<bool> {
    /// The element type
    using ElementType = bool;

    /// false as bool is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST bool type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.bool_(); }
    /// @param b the ProgramBuilder
    /// @return the semantic bool type
    static inline const type::Type* Sem(ProgramBuilder& b) { return b.create<type::Bool>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the boolean value to init with
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<bool>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to bool.
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "bool"; }
};

/// Helper for building i32 types and expressions
template <>
struct DataType<i32> {
    /// The element type
    using ElementType = i32;

    /// false as i32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST i32 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.i32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic i32 type
    static inline const type::Type* Sem(ProgramBuilder& b) { return b.create<type::I32>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the i32 value to init with
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<i32>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to i32.
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "i32"; }
};

/// Helper for building u32 types and expressions
template <>
struct DataType<u32> {
    /// The element type
    using ElementType = u32;

    /// false as u32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST u32 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.u32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic u32 type
    static inline const type::Type* Sem(ProgramBuilder& b) { return b.create<type::U32>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the u32 value to init with
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<u32>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to u32.
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "u32"; }
};

/// Helper for building f32 types and expressions
template <>
struct DataType<f32> {
    /// The element type
    using ElementType = f32;

    /// false as f32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST f32 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.f32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f32 type
    static inline const type::Type* Sem(ProgramBuilder& b) { return b.create<type::F32>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the f32 value to init with
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<f32>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to f32.
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<f32>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "f32"; }
};

/// Helper for building f16 types and expressions
template <>
struct DataType<f16> {
    /// The element type
    using ElementType = f16;

    /// false as f16 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST f16 type
    static inline ast::Type AST(ProgramBuilder& b) { return b.ty.f16(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f16 type
    static inline const type::Type* Sem(ProgramBuilder& b) { return b.create<type::F16>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the f16 value to init with
    /// @return a new AST f16 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<f16>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to f16.
    /// @return a new AST f16 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "f16"; }
};

/// Helper for building abstract float types and expressions
template <>
struct DataType<AFloat> {
    /// The element type
    using ElementType = AFloat;

    /// false as AFloat is not a composite type
    static constexpr bool is_composite = false;

    /// @returns nullptr, as abstract floats are un-typeable
    static inline ast::Type AST(ProgramBuilder&) { return {}; }
    /// @param b the ProgramBuilder
    /// @return the semantic abstract-float type
    static inline const type::Type* Sem(ProgramBuilder& b) {
        return b.create<type::AbstractFloat>();
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the abstract-float value to init with
    /// @return a new AST abstract-float literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<AFloat>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to AFloat.
    /// @return a new AST abstract-float literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "abstract-float"; }
};

/// Helper for building abstract integer types and expressions
template <>
struct DataType<AInt> {
    /// The element type
    using ElementType = AInt;

    /// false as AFloat is not a composite type
    static constexpr bool is_composite = false;

    /// @returns nullptr, as abstract integers are un-typeable
    static inline ast::Type AST(ProgramBuilder&) { return {}; }
    /// @param b the ProgramBuilder
    /// @return the semantic abstract-int type
    static inline const type::Type* Sem(ProgramBuilder& b) { return b.create<type::AbstractInt>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the abstract-int value to init with
    /// @return a new AST abstract-int literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Expr(std::get<AInt>(args[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to AInt.
    /// @return a new AST abstract-int literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() { return "abstract-int"; }
};

/// Helper for building vector types and expressions
template <uint32_t N, typename T>
struct DataType<vec<N, T>> {
    /// The element type
    using ElementType = T;

    /// true as vectors are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST vector type
    static inline ast::Type AST(ProgramBuilder& b) {
        if (IsInferOrAbstract<T>) {
            return b.ty.vec<Infer, N>();
        } else {
            return b.ty.vec(DataType<T>::AST(b), N);
        }
    }
    /// @param b the ProgramBuilder
    /// @return the semantic vector type
    static inline const type::Type* Sem(ProgramBuilder& b) {
        return b.create<type::Vector>(DataType<T>::Sem(b), N);
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return a new AST vector value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Call(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return the list of expressions that are used to construct the vector
    static inline auto ExprArgs(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        const bool one_value = args.Length() == 1;
        utils::Vector<const ast::Expression*, N> r;
        for (size_t i = 0; i < N; ++i) {
            r.Push(DataType<T>::Expr(b, utils::Vector<Scalar, 1>{one_value ? args[0] : args[i]}));
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST vector value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() {
        return "vec" + std::to_string(N) + "<" + DataType<T>::Name() + ">";
    }
};

/// Helper for building matrix types and expressions
template <uint32_t N, uint32_t M, typename T>
struct DataType<mat<N, M, T>> {
    /// The element type
    using ElementType = T;

    /// true as matrices are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST matrix type
    static inline ast::Type AST(ProgramBuilder& b) {
        if (IsInferOrAbstract<T>) {
            return b.ty.mat<Infer, N, M>();
        } else {
            return b.ty.mat(DataType<T>::AST(b), N, M);
        }
    }
    /// @param b the ProgramBuilder
    /// @return the semantic matrix type
    static inline const type::Type* Sem(ProgramBuilder& b) {
        auto* column_type = b.create<type::Vector>(DataType<T>::Sem(b), M);
        return b.create<type::Matrix>(column_type, N);
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N*M with values of type T to initialize with
    /// @return a new AST matrix value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Call(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N*M with values of type T to initialize with
    /// @return a new AST matrix value expression
    static inline auto ExprArgs(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        const bool one_value = args.Length() == 1;
        size_t next = 0;
        utils::Vector<const ast::Expression*, N> r;
        for (uint32_t i = 0; i < N; ++i) {
            if (one_value) {
                r.Push(DataType<vec<M, T>>::Expr(b, utils::Vector<Scalar, 1>{args[0]}));
            } else {
                utils::Vector<Scalar, M> v;
                for (size_t j = 0; j < M; ++j) {
                    v.Push(args[next++]);
                }
                r.Push(DataType<vec<M, T>>::Expr(b, std::move(v)));
            }
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST matrix value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() {
        return "mat" + std::to_string(N) + "x" + std::to_string(M) + "<" + DataType<T>::Name() +
               ">";
    }
};

/// Helper for building alias types and expressions
template <typename T, int ID>
struct DataType<alias<T, ID>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true if the aliased type is a composite type
    static constexpr bool is_composite = DataType<T>::is_composite;

    /// @param b the ProgramBuilder
    /// @return a new AST alias type
    static inline ast::Type AST(ProgramBuilder& b) {
        auto name = b.Symbols().Register("alias_" + std::to_string(ID));
        if (!b.AST().LookupType(name)) {
            auto type = DataType<T>::AST(b);
            b.AST().AddTypeDecl(b.ty.alias(name, type));
        }
        return b.ty(name);
    }

    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const type::Type* Sem(ProgramBuilder& b) { return DataType<T>::Sem(b); }

    /// @param b the ProgramBuilder
    /// @param args the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline utils::traits::EnableIf<!IS_COMPOSITE, const ast::Expression*> Expr(
        ProgramBuilder& b,
        utils::VectorRef<Scalar> args) {
        // Cast
        return b.Call(AST(b), DataType<T>::Expr(b, std::move(args)));
    }

    /// @param b the ProgramBuilder
    /// @param args the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline utils::traits::EnableIf<IS_COMPOSITE, const ast::Expression*> Expr(
        ProgramBuilder& b,
        utils::VectorRef<Scalar> args) {
        // Construct
        return b.Call(AST(b), DataType<T>::ExprArgs(b, std::move(args)));
    }

    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST expression of the alias type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }

    /// @returns the WGSL name for the type
    static inline std::string Name() { return "alias_" + std::to_string(ID); }
};

/// Helper for building pointer types and expressions
template <typename T>
struct DataType<ptr<T>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true if the pointer type is a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST alias type
    static inline ast::Type AST(ProgramBuilder& b) {
        return b.ty.pointer(DataType<T>::AST(b), builtin::AddressSpace::kPrivate,
                            builtin::Access::kUndefined);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const type::Type* Sem(ProgramBuilder& b) {
        return b.create<type::Pointer>(DataType<T>::Sem(b), builtin::AddressSpace::kPrivate,
                                       builtin::Access::kReadWrite);
    }

    /// @param b the ProgramBuilder
    /// @return a new AST expression of the pointer type
    static inline const ast::Expression* Expr(ProgramBuilder& b,
                                              utils::VectorRef<Scalar> /*unused*/) {
        auto sym = b.Symbols().New("global_for_ptr");
        b.GlobalVar(sym, DataType<T>::AST(b), builtin::AddressSpace::kPrivate);
        return b.AddressOf(sym);
    }

    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST expression of the pointer type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }

    /// @returns the WGSL name for the type
    static inline std::string Name() { return "ptr<" + DataType<T>::Name() + ">"; }
};

/// Helper for building array types and expressions
template <uint32_t N, typename T>
struct DataType<array<N, T>> {
    /// The element type
    using ElementType = typename DataType<T>::ElementType;

    /// true as arrays are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST array type
    static inline ast::Type AST(ProgramBuilder& b) {
        if (auto ast = DataType<T>::AST(b)) {
            return b.ty.array(ast, u32(N));
        }
        return b.ty.array<Infer>();
    }
    /// @param b the ProgramBuilder
    /// @return the semantic array type
    static inline const type::Type* Sem(ProgramBuilder& b) {
        auto* el = DataType<T>::Sem(b);
        const type::ArrayCount* count = nullptr;
        if (N == 0) {
            count = b.create<type::RuntimeArrayCount>();
        } else {
            count = b.create<type::ConstantArrayCount>(N);
        }
        return b.create<type::Array>(
            /* element */ el,
            /* count */ count,
            /* align */ el->Align(),
            /* size */ N * el->Size(),
            /* stride */ el->Align(),
            /* implicit_stride */ el->Align());
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// with
    /// @return a new AST array value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        return b.Call(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return the list of expressions that are used to construct the array
    static inline auto ExprArgs(ProgramBuilder& b, utils::VectorRef<Scalar> args) {
        const bool one_value = args.Length() == 1;
        utils::Vector<const ast::Expression*, N> r;
        for (uint32_t i = 0; i < N; i++) {
            r.Push(DataType<T>::Expr(b, utils::Vector<Scalar, 1>{one_value ? args[0] : args[i]}));
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST array value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, utils::Vector<Scalar, 1>{static_cast<ElementType>(v)});
    }
    /// @returns the WGSL name for the type
    static inline std::string Name() {
        return "array<" + DataType<T>::Name() + ", " + std::to_string(N) + ">";
    }
};

/// Struct of all creation pointer types
struct CreatePtrs {
    /// ast node type create function
    ast_type_func_ptr ast;
    /// ast expression type create function
    ast_expr_func_ptr expr;
    /// ast expression type create function from double arg
    ast_expr_from_double_func_ptr expr_from_double;
    /// sem type create function
    sem_type_func_ptr sem;
    /// type name function
    type_name_func_ptr name;
};

/// @param o the std::ostream to write to
/// @param ptrs the CreatePtrs
/// @return the std::ostream so calls can be chained
inline std::ostream& operator<<(std::ostream& o, const CreatePtrs& ptrs) {
    return o << (ptrs.name ? ptrs.name() : "<unknown>");
}

/// Returns a CreatePtrs struct instance with all creation pointer types for
/// type `T`
template <typename T>
constexpr CreatePtrs CreatePtrsFor() {
    return {DataType<T>::AST, DataType<T>::Expr, DataType<T>::ExprFromDouble, DataType<T>::Sem,
            DataType<T>::Name};
}

/// True if DataType<T> is specialized for T, false otherwise.
template <typename T>
const bool IsDataTypeSpecializedFor =
    !std::is_same_v<typename DataType<T>::ElementType, UnspecializedElementType>;

/// Value is used to create Values with a Scalar vector initializer.
struct Value {
    /// Creates a Value for type T initialized with `args`
    /// @param args the scalar args
    /// @returns Value
    template <typename T>
    static Value Create(utils::VectorRef<Scalar> args) {
        static_assert(IsDataTypeSpecializedFor<T>, "No DataType<T> specialization exists");
        using EL_TY = typename builder::DataType<T>::ElementType;
        return Value{
            std::move(args),          //
            CreatePtrsFor<T>(),       //
            tint::IsAbstract<EL_TY>,  //
            tint::IsIntegral<EL_TY>,  //
            tint::FriendlyName<EL_TY>(),
        };
    }

    /// Creates an `ast::Expression` for the type T passing in previously stored args
    /// @param b the ProgramBuilder
    /// @returns an expression node
    const ast::Expression* Expr(ProgramBuilder& b) const { return (*create_ptrs.expr)(b, args); }

    /// Prints this value to the output stream
    /// @param o the output stream
    /// @returns input argument `o`
    std::ostream& Print(std::ostream& o) const {
        o << type_name << "(";
        for (auto& a : args) {
            std::visit([&](auto& v) { o << v; }, a);
            if (&a != &args.Back()) {
                o << ", ";
            }
        }
        o << ")";
        return o;
    }

    /// The arguments used to construct the value
    utils::Vector<Scalar, 4> args;
    /// CreatePtrs for value's type used to create an expression with `args`
    builder::CreatePtrs create_ptrs;
    /// True if the element type is abstract
    bool is_abstract = false;
    /// True if the element type is an integer
    bool is_integral = false;
    /// The name of the type.
    const char* type_name = "<invalid>";
};

/// Prints Value to ostream
inline std::ostream& operator<<(std::ostream& o, const Value& value) {
    return value.Print(o);
}

/// True if T is Value, false otherwise
template <typename T>
constexpr bool IsValue = std::is_same_v<T, Value>;

/// Creates a Value of DataType<T> from a scalar `v`
template <typename T>
Value Val(T v) {
    static_assert(utils::traits::IsTypeIn<T, Scalar>, "v must be a Number of bool");
    return Value::Create<T>(utils::Vector<Scalar, 1>{v});
}

/// Creates a Value of DataType<vec<N, T>> from N scalar `args`
template <typename... Ts>
Value Vec(Ts... args) {
    using FirstT = std::tuple_element_t<0, std::tuple<Ts...>>;
    static_assert(sizeof...(args) >= 2 && sizeof...(args) <= 4, "Invalid vector size");
    static_assert(std::conjunction_v<std::is_same<FirstT, Ts>...>,
                  "Vector args must all be the same type");
    constexpr size_t N = sizeof...(args);
    utils::Vector<Scalar, sizeof...(args)> v{args...};
    return Value::Create<vec<N, FirstT>>(std::move(v));
}

/// Creates a Value of DataType<array<N, T>> from N scalar `args`
template <typename... Ts>
Value Array(Ts... args) {
    using FirstT = std::tuple_element_t<0, std::tuple<Ts...>>;
    static_assert(std::conjunction_v<std::is_same<FirstT, Ts>...>,
                  "Array args must all be the same type");
    constexpr size_t N = sizeof...(args);
    utils::Vector<Scalar, sizeof...(args)> v{args...};
    return Value::Create<array<N, FirstT>>(std::move(v));
}

/// Creates a Value of DataType<mat<C,R,T> from C*R scalar `args`
template <size_t C, size_t R, typename T>
Value Mat(const T (&m_in)[C][R]) {
    utils::Vector<Scalar, C * R> m;
    for (uint32_t i = 0; i < C; ++i) {
        for (size_t j = 0; j < R; ++j) {
            m.Push(m_in[i][j]);
        }
    }
    return Value::Create<mat<C, R, T>>(std::move(m));
}

/// Creates a Value of DataType<mat<2,R,T> from column vectors `c0` and `c1`
template <typename T, size_t R>
Value Mat(const T (&c0)[R], const T (&c1)[R]) {
    constexpr size_t C = 2;
    utils::Vector<Scalar, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    return Value::Create<mat<C, R, T>>(std::move(m));
}

/// Creates a Value of DataType<mat<3,R,T> from column vectors `c0`, `c1`, and `c2`
template <typename T, size_t R>
Value Mat(const T (&c0)[R], const T (&c1)[R], const T (&c2)[R]) {
    constexpr size_t C = 3;
    utils::Vector<Scalar, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    for (auto v : c2) {
        m.Push(v);
    }
    return Value::Create<mat<C, R, T>>(std::move(m));
}

/// Creates a Value of DataType<mat<4,R,T> from column vectors `c0`, `c1`, `c2`, and `c3`
template <typename T, size_t R>
Value Mat(const T (&c0)[R], const T (&c1)[R], const T (&c2)[R], const T (&c3)[R]) {
    constexpr size_t C = 4;
    utils::Vector<Scalar, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    for (auto v : c2) {
        m.Push(v);
    }
    for (auto v : c3) {
        m.Push(v);
    }
    return Value::Create<mat<C, R, T>>(std::move(m));
}
}  // namespace builder
}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_RESOLVER_TEST_HELPER_H_
