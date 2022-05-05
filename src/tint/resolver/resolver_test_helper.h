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

#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

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
                       std::vector<const ast::Expression*>&& expected_users) {
        auto& var_users = Sem().Get(var)->Users();
        if (var_users.size() != expected_users.size()) {
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

using ast_type_func_ptr = const ast::Type* (*)(ProgramBuilder& b);
using ast_expr_func_ptr = const ast::Expression* (*)(ProgramBuilder& b, int elem_value);
using sem_type_func_ptr = const sem::Type* (*)(ProgramBuilder& b);

template <typename T>
struct DataType {};

/// Helper for building bool types and expressions
template <>
struct DataType<bool> {
    /// false as bool is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST bool type
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.bool_(); }
    /// @param b the ProgramBuilder
    /// @return the semantic bool type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::Bool>(); }
    /// @param b the ProgramBuilder
    /// @param elem_value the b
    /// @return a new AST expression of the bool type
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Expr(elem_value == 0);
    }
};

/// Helper for building i32 types and expressions
template <>
struct DataType<i32> {
    /// false as i32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST i32 type
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.i32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic i32 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::I32>(); }
    /// @param b the ProgramBuilder
    /// @param elem_value the value i32 will be initialized with
    /// @return a new AST i32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Expr(static_cast<i32>(elem_value));
    }
};

/// Helper for building u32 types and expressions
template <>
struct DataType<u32> {
    /// false as u32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST u32 type
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.u32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic u32 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::U32>(); }
    /// @param b the ProgramBuilder
    /// @param elem_value the value u32 will be initialized with
    /// @return a new AST u32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Expr(static_cast<u32>(elem_value));
    }
};

/// Helper for building f32 types and expressions
template <>
struct DataType<f32> {
    /// false as f32 is not a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST f32 type
    static inline const ast::Type* AST(ProgramBuilder& b) { return b.ty.f32(); }
    /// @param b the ProgramBuilder
    /// @return the semantic f32 type
    static inline const sem::Type* Sem(ProgramBuilder& b) { return b.create<sem::F32>(); }
    /// @param b the ProgramBuilder
    /// @param elem_value the value f32 will be initialized with
    /// @return a new AST f32 literal value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Expr(static_cast<f32>(elem_value));
    }
};

/// Helper for building vector types and expressions
template <uint32_t N, typename T>
struct DataType<vec<N, T>> {
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
    /// @param elem_value the value each element in the vector will be initialized
    /// with
    /// @return a new AST vector value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Construct(AST(b), ExprArgs(b, elem_value));
    }

    /// @param b the ProgramBuilder
    /// @param elem_value the value each element will be initialized with
    /// @return the list of expressions that are used to construct the vector
    static inline ast::ExpressionList ExprArgs(ProgramBuilder& b, int elem_value) {
        ast::ExpressionList args;
        for (uint32_t i = 0; i < N; i++) {
            args.emplace_back(DataType<T>::Expr(b, elem_value));
        }
        return args;
    }
};

/// Helper for building matrix types and expressions
template <uint32_t N, uint32_t M, typename T>
struct DataType<mat<N, M, T>> {
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
    /// @param elem_value the value each element in the matrix will be initialized
    /// with
    /// @return a new AST matrix value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Construct(AST(b), ExprArgs(b, elem_value));
    }

    /// @param b the ProgramBuilder
    /// @param elem_value the value each element will be initialized with
    /// @return the list of expressions that are used to construct the matrix
    static inline ast::ExpressionList ExprArgs(ProgramBuilder& b, int elem_value) {
        ast::ExpressionList args;
        for (uint32_t i = 0; i < N; i++) {
            args.emplace_back(DataType<vec<M, T>>::Expr(b, elem_value));
        }
        return args;
    }
};

/// Helper for building alias types and expressions
template <typename T, int ID>
struct DataType<alias<T, ID>> {
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
    /// @param elem_value the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline traits::EnableIf<!IS_COMPOSITE, const ast::Expression*> Expr(ProgramBuilder& b,
                                                                               int elem_value) {
        // Cast
        return b.Construct(AST(b), DataType<T>::Expr(b, elem_value));
    }

    /// @param b the ProgramBuilder
    /// @param elem_value the value nested elements will be initialized with
    /// @return a new AST expression of the alias type
    template <bool IS_COMPOSITE = is_composite>
    static inline traits::EnableIf<IS_COMPOSITE, const ast::Expression*> Expr(ProgramBuilder& b,
                                                                              int elem_value) {
        // Construct
        return b.Construct(AST(b), DataType<T>::ExprArgs(b, elem_value));
    }
};

/// Helper for building pointer types and expressions
template <typename T>
struct DataType<ptr<T>> {
    /// true if the pointer type is a composite type
    static constexpr bool is_composite = false;

    /// @param b the ProgramBuilder
    /// @return a new AST alias type
    static inline const ast::Type* AST(ProgramBuilder& b) {
        return b.create<ast::Pointer>(DataType<T>::AST(b), ast::StorageClass::kPrivate,
                                      ast::Access::kReadWrite);
    }
    /// @param b the ProgramBuilder
    /// @return the semantic aliased type
    static inline const sem::Type* Sem(ProgramBuilder& b) {
        return b.create<sem::Pointer>(DataType<T>::Sem(b), ast::StorageClass::kPrivate,
                                      ast::Access::kReadWrite);
    }

    /// @param b the ProgramBuilder
    /// @return a new AST expression of the alias type
    static inline const ast::Expression* Expr(ProgramBuilder& b, int /*unused*/) {
        auto sym = b.Symbols().New("global_for_ptr");
        b.Global(sym, DataType<T>::AST(b), ast::StorageClass::kPrivate);
        return b.AddressOf(sym);
    }
};

/// Helper for building array types and expressions
template <uint32_t N, typename T>
struct DataType<array<N, T>> {
    /// true as arrays are a composite type
    static constexpr bool is_composite = true;

    /// @param b the ProgramBuilder
    /// @return a new AST array type
    static inline const ast::Type* AST(ProgramBuilder& b) {
        return b.ty.array(DataType<T>::AST(b), u32(N));
    }
    /// @param b the ProgramBuilder
    /// @return the semantic array type
    static inline const sem::Type* Sem(ProgramBuilder& b) {
        auto* el = DataType<T>::Sem(b);
        return b.create<sem::Array>(
            /* element */ el,
            /* count */ N,
            /* align */ el->Align(),
            /* size */ el->Size(),
            /* stride */ el->Align(),
            /* implicit_stride */ el->Align());
    }
    /// @param b the ProgramBuilder
    /// @param elem_value the value each element in the array will be initialized
    /// with
    /// @return a new AST array value expression
    static inline const ast::Expression* Expr(ProgramBuilder& b, int elem_value) {
        return b.Construct(AST(b), ExprArgs(b, elem_value));
    }

    /// @param b the ProgramBuilder
    /// @param elem_value the value each element will be initialized with
    /// @return the list of expressions that are used to construct the array
    static inline ast::ExpressionList ExprArgs(ProgramBuilder& b, int elem_value) {
        ast::ExpressionList args;
        for (uint32_t i = 0; i < N; i++) {
            args.emplace_back(DataType<T>::Expr(b, elem_value));
        }
        return args;
    }
};

/// Struct of all creation pointer types
struct CreatePtrs {
    /// ast node type create function
    ast_type_func_ptr ast;
    /// ast expression type create function
    ast_expr_func_ptr expr;
    /// sem type create function
    sem_type_func_ptr sem;
};

/// Returns a CreatePtrs struct instance with all creation pointer types for
/// type `T`
template <typename T>
constexpr CreatePtrs CreatePtrsFor() {
    return {DataType<T>::AST, DataType<T>::Expr, DataType<T>::Sem};
}

}  // namespace builder

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_RESOLVER_TEST_HELPER_H_
