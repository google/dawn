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
#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
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
        auto* sem_ident = Sem().Get(expr);
        auto* var_user = sem_ident ? sem_ident->As<sem::VariableUser>() : nullptr;
        return var_user ? var_user->Variable() : nullptr;
    }

    /// Checks that all the users of the given variable are as expected
    /// @param var the variable to check
    /// @param expected_users the expected users of the variable
    /// @return true if all users are as expected
    bool CheckVarUsers(const ast::Variable* var,
                       utils::VectorRef<const ast::Expression*> expected_users) {
        auto& var_users = Sem().Get(var)->Users();
        if (var_users.size() != expected_users.Length()) {
            return false;
        }
        for (size_t i = 0; i < var_users.size(); i++) {
            if (var_users[i]->Declaration() != expected_users[i]) {
                return false;
            }
        }
        return true;
    }

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const ast::Type* type) { return type->FriendlyName(Symbols()); }

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const sem::Type* type) { return type->FriendlyName(Symbols()); }

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
using mat3x2 = mat<3, 2, T>;

template <typename T>
using mat2x4 = mat<2, 4, T>;

template <typename T>
using mat4x2 = mat<4, 2, T>;

template <typename T>
using mat3x3 = mat<3, 3, T>;

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

/// Type used to accept scalars as arguments. Can be either a single value that gets splatted for
/// composite types, or all values required by the composite type.
struct ScalarArgs {
    /// Constructor
    ScalarArgs() = default;

    /// Constructor
    /// @param single_value single value to initialize with
    template <typename T>
    explicit ScalarArgs(T single_value) : values(utils::Vector<Storage, 1>{single_value}) {}

    /// Constructor
    /// @param all_values all values to initialize the composite type with
    template <typename T>
    ScalarArgs(utils::VectorRef<T> all_values)  // NOLINT: implicit on purpose
    {
        for (auto& v : all_values) {
            values.Push(v);
        }
    }

    /// @param other the other ScalarArgs to compare against
    /// @returns true if all values are equal to the values in @p other
    bool operator==(const ScalarArgs& other) const { return values == other.values; }

    /// Valid scalar types for args
    using Storage = std::variant<i32, u32, f32, f16, AInt, AFloat, bool>;

    /// The vector of values
    utils::Vector<Storage, 16> values;
};

/// Returns current variant value in `s` cast to type `T`
template <typename T>
T As(ScalarArgs::Storage& s) {
    return std::visit([](auto&& v) { return static_cast<T>(v); }, s);
}

/// @param o the std::ostream to write to
/// @param args the ScalarArgs
/// @return the std::ostream so calls can be chained
inline std::ostream& operator<<(std::ostream& o, const ScalarArgs& args) {
    o << "[";
    bool first = true;
    for (auto& val : args.values) {
        if (!first) {
            o << ", ";
        }
        first = false;
        std::visit([&](auto&& v) { o << v; }, val);
    }
    o << "]";
    return o;
}

using ast_type_func_ptr = const ast::Type* (*)(ProgramBuilder& b);
using ast_expr_func_ptr = const ast::Expression* (*)(ProgramBuilder& b, ScalarArgs args);
using ast_expr_from_double_func_ptr = const ast::Expression* (*)(ProgramBuilder& b, double v);
using sem_type_func_ptr = const sem::Type* (*)(ProgramBuilder& b);
using type_name_func_ptr = std::string (*)();

template <typename T>
struct DataType {};

/// Helper that represents no-type. Returns nullptr for all static methods.
template <>
struct DataType<void> {
    /// @return nullptr
    static inline const ast::Type* AST(ProgramBuilder&) { return nullptr; }
    /// @return nullptr
    static inline const sem::Type* Sem(ProgramBuilder&) { return nullptr; }
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
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.bool_(); }
    /// @param b the ProgramBuilder
    /// @return the semantic bool type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::Bool>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the boolean value to init with
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<bool>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to bool.
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.i32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic i32 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::I32>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the i32 value to init with
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<i32>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to i32.
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.u32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic u32 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::U32>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the u32 value to init with
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<u32>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to u32.
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.f32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f32 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::F32>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the f32 value to init with
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<f32>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to f32.
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<f32>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.f16(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f16 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::F16>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the f16 value to init with
    /// @return a new AST f16 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<f16>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to f16.
    /// @return a new AST f16 literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder&) { return nullptr; }
    /// @param b the ProgramBuilder
    /// @return the semantic abstract-float type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::AbstractFloat>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the abstract-float value to init with
    /// @return a new AST abstract-float literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<AFloat>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to AFloat.
    /// @return a new AST abstract-float literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder&) { return nullptr; }
    /// @param b the ProgramBuilder
    /// @return the semantic abstract-int type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::AbstractInt>(); }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 with the abstract-int value to init with
    /// @return a new AST abstract-int literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Expr(std::get<AInt>(args.values[0]));
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to AInt.
    /// @return a new AST abstract-int literal value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) {
        return b.ty.vec(DataType<T>::AST(b), N);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic vector type
    static inline const sem::Type* Sem(ProgramBuilder& b) {
        return b.create<sem::Vector>(DataType<T>::Sem(b), N);
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return a new AST vector value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Construct(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return the list of expressions that are used to construct the vector
    static inline auto ExprArgs(ProgramBuilder& b, ScalarArgs args) {
        const bool one_value = args.values.Length() == 1;
        utils::Vector<const ast::Expression*, N> r;
        for (size_t i = 0; i < N; ++i) {
            r.Push(DataType<T>::Expr(b, ScalarArgs{one_value ? args.values[0] : args.values[i]}));
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST vector value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) {
        return b.ty.mat(DataType<T>::AST(b), N, M);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic matrix type
    static inline const sem::Type* Sem(ProgramBuilder& b) {
        auto* column_type = b.create<sem::Vector>(DataType<T>::Sem(b), M);
        return b.create<sem::Matrix>(column_type, N);
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N*M with values of type T to initialize with
    /// @return a new AST matrix value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Construct(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N*M with values of type T to initialize with
    /// @return a new AST matrix value expression
    static inline auto ExprArgs(ProgramBuilder& b, ScalarArgs args) {
        const bool one_value = args.values.Length() == 1;
        size_t next = 0;
        utils::Vector<const ast::Expression*, N> r;
        for (uint32_t i = 0; i < N; ++i) {
            if (one_value) {
                r.Push(DataType<vec<M, T>>::Expr(b, ScalarArgs{args.values[0]}));
            } else {
                utils::Vector<T, M> v;
                for (size_t j = 0; j < M; ++j) {
                    v.Push(std::get<T>(args.values[next++]));
                }
                r.Push(DataType<vec<M, T>>::Expr(b, utils::VectorRef<T>{v}));
            }
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST matrix value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) {
        auto name = b.Symbols().Register("alias_" + std::to_string(ID));
        if (!b.AST().LookupType(name)) {
            auto* type = DataType<T>::AST(b);
            b.AST().AddTypeDecl(b.ty.alias(name, type));
        }
        return b.create<ast::TypeName>(name);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return DataType<T>::Sem(b); }

    /// @param b the ProgramBuilder
    /// @param args the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline traits::EnableIf<!IS_COMPOSITE, const ast::Expression*> Expr(ProgramBuilder& b,
                                                                               ScalarArgs args) {
        // Cast
        return b.Construct(AST(b), DataType<T>::Expr(b, std::move(args)));
    }

    /// @param b the ProgramBuilder
    /// @param args the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline traits::EnableIf<IS_COMPOSITE, const ast::Expression*> Expr(ProgramBuilder& b,
                                                                              ScalarArgs args) {
        // Construct
        return b.Construct(AST(b), DataType<T>::ExprArgs(b, std::move(args)));
    }

    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST expression of the alias type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) {
        return b.create<ast::Pointer>(DataType<T>::AST(b), ast::AddressSpace::kPrivate,
                                      ast::Access::kReadWrite);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const sem::Type* Sem(ProgramBuilder& b) {
        return b.create<sem::Pointer>(DataType<T>::Sem(b), ast::AddressSpace::kPrivate,
                                      ast::Access::kReadWrite);
    }

    /// @param b the ProgramBuilder
    /// @return a new AST expression of the pointer type
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs /*unused*/) {
        auto sym = b.Symbols().New("global_for_ptr");
        b.GlobalVar(sym, DataType<T>::AST(b), ast::AddressSpace::kPrivate);
        return b.AddressOf(sym);
    }

    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST expression of the pointer type
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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
    static inline const ast::Type* AST(ProgramBuilder& b) {
        if (auto* ast = DataType<T>::AST(b)) {
            return b.ty.array(ast, u32(N));
        }
        return b.ty.array(nullptr, nullptr);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic array type
    static inline const sem::Type* Sem(ProgramBuilder& b) {
        auto* el = DataType<T>::Sem(b);
        sem::ArrayCount count = sem::ConstantArrayCount{N};
        if (N == 0) {
            count = sem::RuntimeArrayCount{};
        }
        return b.create<sem::Array>(
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
    static inline const ast::Expression* Expr(ProgramBuilder& b, ScalarArgs args) {
        return b.Construct(AST(b), ExprArgs(b, std::move(args)));
    }
    /// @param b the ProgramBuilder
    /// @param args args of size 1 or N with values of type T to initialize with
    /// @return the list of expressions that are used to construct the array
    static inline auto ExprArgs(ProgramBuilder& b, ScalarArgs args) {
        const bool one_value = args.values.Length() == 1;
        utils::Vector<const ast::Expression*, N> r;
        for (uint32_t i = 0; i < N; i++) {
            r.Push(DataType<T>::Expr(b, ScalarArgs{one_value ? args.values[0] : args.values[i]}));
        }
        return r;
    }
    /// @param b the ProgramBuilder
    /// @param v arg of type double that will be cast to ElementType
    /// @return a new AST array value expression
    static inline const ast::Expression* ExprFromDouble(ProgramBuilder& b, double v) {
        return Expr(b, ScalarArgs{static_cast<ElementType>(v)});
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

/// Base class for Value<T>
struct ValueBase {
    /// Constructor
    ValueBase() = default;
    /// Destructor
    virtual ~ValueBase() = default;
    /// Move constructor
    ValueBase(ValueBase&&) = default;
    /// Copy constructor
    ValueBase(const ValueBase&) = default;
    /// Copy assignment operator
    /// @returns this instance
    ValueBase& operator=(const ValueBase&) = default;
    /// Creates an `ast::Expression` for the type T passing in previously stored args
    /// @param b the ProgramBuilder
    /// @returns an expression node
    virtual const ast::Expression* Expr(ProgramBuilder& b) const = 0;
    /// @returns args used to create expression via `Expr`
    virtual const ScalarArgs& Args() const = 0;
    /// @returns true if element type is abstract
    virtual bool IsAbstract() const = 0;
    /// @returns true if element type is an integral
    virtual bool IsIntegral() const = 0;
    /// @returns element type name
    virtual std::string TypeName() const = 0;
    /// Prints this value to the output stream
    /// @param o the output stream
    /// @returns input argument `o`
    virtual std::ostream& Print(std::ostream& o) const = 0;
};

/// Value<T> is an instance of a value of type DataType<T>. Useful for storing values to create
/// expressions with.
template <typename T>
struct Value : ValueBase {
    /// Constructor
    /// @param a the scalar args
    explicit Value(ScalarArgs a) : args(std::move(a)) {}

    /// Alias to T
    using Type = T;
    /// Alias to DataType<T>
    using DataType = builder::DataType<T>;
    /// Alias to DataType::ElementType
    using ElementType = typename DataType::ElementType;

    /// Creates a Value<T> with `args`
    /// @param args the args that will be passed to the expression
    /// @returns a Value<T>
    static Value Create(ScalarArgs args) { return Value{std::move(args)}; }

    /// Creates an `ast::Expression` for the type T passing in previously stored args
    /// @param b the ProgramBuilder
    /// @returns an expression node
    const ast::Expression* Expr(ProgramBuilder& b) const override {
        auto create = CreatePtrsFor<T>();
        return (*create.expr)(b, args);
    }

    /// @returns args used to create expression via `Expr`
    const ScalarArgs& Args() const override { return args; }

    /// @returns true if element type is abstract
    bool IsAbstract() const override { return tint::IsAbstract<ElementType>; }

    /// @returns true if element type is an integral
    bool IsIntegral() const override { return tint::IsIntegral<ElementType>; }

    /// @returns element type name
    std::string TypeName() const override { return tint::FriendlyName<ElementType>(); }

    /// Prints this value to the output stream
    /// @param o the output stream
    /// @returns input argument `o`
    std::ostream& Print(std::ostream& o) const override {
        o << TypeName() << "(";
        for (auto& a : args.values) {
            o << std::get<ElementType>(a);
            if (&a != &args.values.Back()) {
                o << ", ";
            }
        }
        o << ")";
        return o;
    }

    /// args to create expression with
    ScalarArgs args;
};

namespace detail {
/// Base template for IsValue
template <typename T>
struct IsValue : std::false_type {};
/// Specialization for IsValue
template <typename T>
struct IsValue<Value<T>> : std::true_type {};
}  // namespace detail

/// True if T is of type Value
template <typename T>
constexpr bool IsValue = detail::IsValue<T>::value;

/// Returns the friendly name of ValueT
template <typename ValueT, typename = traits::EnableIf<IsValue<ValueT>>>
const char* FriendlyName() {
    return tint::FriendlyName<typename ValueT::ElementType>();
}

/// Creates a `Value<T>` from a scalar `v`
template <typename T>
auto Val(T v) {
    return Value<T>::Create(ScalarArgs{v});
}

/// Creates a `Value<vec<N, T>>` from N scalar `args`
template <typename... T>
auto Vec(T... args) {
    constexpr size_t N = sizeof...(args);
    using FirstT = std::tuple_element_t<0, std::tuple<T...>>;
    utils::Vector v{args...};
    using VT = vec<N, FirstT>;
    return Value<VT>::Create(utils::VectorRef<FirstT>{v});
}

/// Creates a `Value<mat<C,R,T>` from C*R scalar `args`
template <size_t C, size_t R, typename T>
auto Mat(const T (&m_in)[C][R]) {
    utils::Vector<T, C * R> m;
    for (uint32_t i = 0; i < C; ++i) {
        for (size_t j = 0; j < R; ++j) {
            m.Push(m_in[i][j]);
        }
    }
    return Value<mat<C, R, T>>::Create(utils::VectorRef<T>{m});
}

/// Creates a `Value<mat<2,R,T>` from column vectors `c0` and `c1`
template <typename T, size_t R>
auto Mat(const T (&c0)[R], const T (&c1)[R]) {
    constexpr size_t C = 2;
    utils::Vector<T, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    return Value<mat<C, R, T>>::Create(utils::VectorRef<T>{m});
}

/// Creates a `Value<mat<3,R,T>` from column vectors `c0`, `c1`, and `c2`
template <typename T, size_t R>
auto Mat(const T (&c0)[R], const T (&c1)[R], const T (&c2)[R]) {
    constexpr size_t C = 3;
    utils::Vector<T, C * R> m;
    for (auto v : c0) {
        m.Push(v);
    }
    for (auto v : c1) {
        m.Push(v);
    }
    for (auto v : c2) {
        m.Push(v);
    }
    return Value<mat<C, R, T>>::Create(utils::VectorRef<T>{m});
}

/// Creates a `Value<mat<4,R,T>` from column vectors `c0`, `c1`, `c2`, and `c3`
template <typename T, size_t R>
auto Mat(const T (&c0)[R], const T (&c1)[R], const T (&c2)[R], const T (&c3)[R]) {
    constexpr size_t C = 4;
    utils::Vector<T, C * R> m;
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
    return Value<mat<C, R, T>>::Create(utils::VectorRef<T>{m});
}

}  // namespace builder
}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_RESOLVER_TEST_HELPER_H_
