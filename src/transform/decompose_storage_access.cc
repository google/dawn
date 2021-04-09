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

#include "src/transform/decompose_storage_access.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/assignment_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/program_builder.h"
#include "src/semantic/array.h"
#include "src/semantic/call.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/struct.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"
#include "src/utils/get_or_create.h"
#include "src/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::DecomposeStorageAccess::Intrinsic);

namespace tint {
namespace transform {

namespace {

/// Offset is a simple ast::Expression builder interface, used to build byte
/// offsets for storage buffer accesses.
struct Offset : Castable<Offset> {
  /// @returns builds and returns the ast::Expression in `ctx.dst`
  virtual ast::Expression* Build(CloneContext& ctx) = 0;
};

/// OffsetExpr is an implementation of Offset that clones and casts the given
/// expression to `u32`.
struct OffsetExpr : Offset {
  ast::Expression* const expr = nullptr;

  explicit OffsetExpr(ast::Expression* e) : expr(e) {}

  ast::Expression* Build(CloneContext& ctx) override {
    auto* type = ctx.src->Sem().Get(expr)->Type()->UnwrapAll();
    auto* res = ctx.Clone(expr);
    if (!type->Is<type::U32>()) {
      res = ctx.dst->Construct<ProgramBuilder::u32>(res);
    }
    return res;
  }
};

/// OffsetLiteral is an implementation of Offset that constructs a u32 literal
/// value.
struct OffsetLiteral : Castable<OffsetLiteral, Offset> {
  uint32_t const literal = 0;

  explicit OffsetLiteral(uint32_t lit) : literal(lit) {}

  ast::Expression* Build(CloneContext& ctx) override {
    return ctx.dst->Expr(literal);
  }
};

/// OffsetBinOp is an implementation of Offset that constructs a binary-op of
/// two Offsets.
struct OffsetBinOp : Offset {
  ast::BinaryOp op;
  std::unique_ptr<Offset> lhs;
  std::unique_ptr<Offset> rhs;

  ast::Expression* Build(CloneContext& ctx) override {
    return ctx.dst->create<ast::BinaryExpression>(op, lhs->Build(ctx),
                                                  rhs->Build(ctx));
  }
};

/// @returns an Offset for the given literal value
std::unique_ptr<Offset> ToOffset(uint32_t offset) {
  return std::make_unique<OffsetLiteral>(offset);
}

/// @returns an Offset for the given ast::Expression
std::unique_ptr<Offset> ToOffset(ast::Expression* expr) {
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    if (auto* u32 = scalar->literal()->As<ast::UintLiteral>()) {
      return std::make_unique<OffsetLiteral>(u32->value());
    } else if (auto* i32 = scalar->literal()->As<ast::SintLiteral>()) {
      if (i32->value() > 0) {
        return std::make_unique<OffsetLiteral>(i32->value());
      }
    }
  }
  return std::make_unique<OffsetExpr>(expr);
}

/// @returns the given offset (pass-through)
std::unique_ptr<Offset> ToOffset(std::unique_ptr<Offset> offset) {
  return offset;
}

/// @return an Offset that is a sum of lhs and rhs, performing basic constant
/// folding if possible
template <typename LHS, typename RHS>
std::unique_ptr<Offset> Add(LHS&& lhs_, RHS&& rhs_) {
  std::unique_ptr<Offset> lhs = ToOffset(std::forward<LHS>(lhs_));
  std::unique_ptr<Offset> rhs = ToOffset(std::forward<RHS>(rhs_));
  auto* lhs_lit = lhs->As<OffsetLiteral>();
  auto* rhs_lit = rhs->As<OffsetLiteral>();
  if (lhs_lit && lhs_lit->literal == 0) {
    return rhs;
  }
  if (rhs_lit && rhs_lit->literal == 0) {
    return lhs;
  }
  if (lhs_lit && rhs_lit) {
    if (static_cast<uint64_t>(lhs_lit->literal) +
            static_cast<uint64_t>(rhs_lit->literal) <=
        0xffffffff) {
      return std::make_unique<OffsetLiteral>(lhs_lit->literal +
                                             rhs_lit->literal);
    }
  }
  auto out = std::make_unique<OffsetBinOp>();
  out->op = ast::BinaryOp::kAdd;
  out->lhs = std::move(lhs);
  out->rhs = std::move(rhs);
  return out;
}

/// @return an Offset that is the multiplication of lhs and rhs, performing
/// basic constant folding if possible
template <typename LHS, typename RHS>
std::unique_ptr<Offset> Mul(LHS&& lhs_, RHS&& rhs_) {
  std::unique_ptr<Offset> lhs = ToOffset(std::forward<LHS>(lhs_));
  std::unique_ptr<Offset> rhs = ToOffset(std::forward<RHS>(rhs_));
  auto* lhs_lit = lhs->As<OffsetLiteral>();
  auto* rhs_lit = rhs->As<OffsetLiteral>();
  if (lhs_lit && lhs_lit->literal == 0) {
    return std::make_unique<OffsetLiteral>(0);
  }
  if (rhs_lit && rhs_lit->literal == 0) {
    return std::make_unique<OffsetLiteral>(0);
  }
  if (lhs_lit && lhs_lit->literal == 1) {
    return rhs;
  }
  if (rhs_lit && rhs_lit->literal == 1) {
    return lhs;
  }
  if (lhs_lit && rhs_lit) {
    return std::make_unique<OffsetLiteral>(lhs_lit->literal * rhs_lit->literal);
  }
  auto out = std::make_unique<OffsetBinOp>();
  out->op = ast::BinaryOp::kMultiply;
  out->lhs = std::move(lhs);
  out->rhs = std::move(rhs);
  return out;
}

/// TypePair is a pair of types that can be used as a unordered map or set key.
struct TypePair {
  type::Type* first;
  type::Type* second;
  bool operator==(const TypePair& rhs) const {
    return first == rhs.first && second == rhs.second;
  }
  struct Hasher {
    inline std::size_t operator()(const TypePair& u) const {
      return utils::Hash(u.first, u.second);
    }
  };
};

/// @returns the size in bytes of a scalar
uint32_t ScalarSize(type::Type*) {
  // TODO(bclayton): Assumes 32-bit elements
  return 4;
}

/// @returns the numer of bytes between columns of the given matrix
uint32_t MatrixColumnStride(type::Matrix* mat) {
  return ScalarSize(mat->type()) * ((mat->rows() == 2) ? 2 : 4);
}

/// @returns a DecomposeStorageAccess::Intrinsic decoration that can be applied
/// to a stub function to load the type `ty`.
DecomposeStorageAccess::Intrinsic* IntrinsicLoadFor(ProgramBuilder* builder,
                                                    type::Type* ty) {
  using Intrinsic = DecomposeStorageAccess::Intrinsic;

  if (ty->Is<type::I32>()) {
    return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadI32);
  }
  if (ty->Is<type::U32>()) {
    return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadU32);
  }
  if (ty->Is<type::F32>()) {
    return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadF32);
  }
  if (auto* vec = ty->As<type::Vector>()) {
    switch (vec->size()) {
      case 2:
        if (vec->type()->Is<type::I32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec2I32);
        }
        if (vec->type()->Is<type::U32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec2U32);
        }
        if (vec->type()->Is<type::F32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec2F32);
        }
        break;
      case 3:
        if (vec->type()->Is<type::I32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec3I32);
        }
        if (vec->type()->Is<type::U32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec3U32);
        }
        if (vec->type()->Is<type::F32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec3F32);
        }
        break;
      case 4:
        if (vec->type()->Is<type::I32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec4I32);
        }
        if (vec->type()->Is<type::U32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec4U32);
        }
        if (vec->type()->Is<type::F32>()) {
          return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kLoadVec4F32);
        }
        break;
    }
  }
  return nullptr;
}

/// @returns a DecomposeStorageAccess::Intrinsic decoration that can be applied
/// to a stub function to store the type `ty`.
DecomposeStorageAccess::Intrinsic* IntrinsicStoreFor(ProgramBuilder* builder,
                                                     type::Type* ty) {
  using Intrinsic = DecomposeStorageAccess::Intrinsic;

  if (ty->Is<type::I32>()) {
    return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kStoreI32);
  }
  if (ty->Is<type::U32>()) {
    return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kStoreU32);
  }
  if (ty->Is<type::F32>()) {
    return builder->ASTNodes().Create<Intrinsic>(Intrinsic::kStoreF32);
  }
  if (auto* vec = ty->As<type::Vector>()) {
    switch (vec->size()) {
      case 2:
        if (vec->type()->Is<type::I32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec2U32);
        }
        if (vec->type()->Is<type::U32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec2F32);
        }
        if (vec->type()->Is<type::F32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec2I32);
        }
        break;
      case 3:
        if (vec->type()->Is<type::I32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec3U32);
        }
        if (vec->type()->Is<type::U32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec3F32);
        }
        if (vec->type()->Is<type::F32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec3I32);
        }
        break;
      case 4:
        if (vec->type()->Is<type::I32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec4U32);
        }
        if (vec->type()->Is<type::U32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec4F32);
        }
        if (vec->type()->Is<type::F32>()) {
          return builder->ASTNodes().Create<Intrinsic>(
              Intrinsic::kStoreVec4I32);
        }
        break;
    }
  }
  return nullptr;
}

/// Inserts `node` before `insert_after` in the global declarations of
/// `ctx.dst`. If `insert_after` is nullptr, then `node` is inserted at the top
/// of the module.
void InsertGlobal(CloneContext& ctx, Cloneable* insert_after, Cloneable* node) {
  auto& globals = ctx.src->AST().GlobalDeclarations();
  if (insert_after) {
    ctx.InsertAfter(globals, insert_after, node);
  } else {
    ctx.InsertBefore(globals, *globals.begin(), node);
  }
}

/// @returns the unwrapped, user-declared constructed type of ty.
type::Type* ConstructedTypeOf(type::Type* ty) {
  while (true) {
    if (auto* ptr = ty->As<type::Pointer>()) {
      ty = ptr->type();
      continue;
    }
    if (auto* access = ty->As<type::AccessControl>()) {
      ty = access->type();
      continue;
    }
    if (auto* alias = ty->As<type::Alias>()) {
      return alias;
    }
    if (auto* str = ty->As<type::Struct>()) {
      return str;
    }
    // Not a constructed type
    return nullptr;
  }
}

/// @returns the given type with all pointers and aliases removed.
type::Type* UnwrapPtrAndAlias(type::Type* ty) {
  return ty->UnwrapPtrIfNeeded()->UnwrapAliasIfNeeded()->UnwrapPtrIfNeeded();
}

/// StorageBufferAccess describes a single storage buffer access
struct StorageBufferAccess {
  semantic::Expression const* var = nullptr;  // Storage buffer variable
  std::unique_ptr<Offset> offset;             // The byte offset on var
  type::Type* type = nullptr;                 // The type of the access
  operator bool() const { return var; }       // Returns true if valid
};

/// Store describes a single storage buffer write
struct Store {
  ast::AssignmentStatement* assignment;  // The AST assignment statement
  StorageBufferAccess target;            // The target for the write
};

/// State holds the current transform state
struct State {
  /// Map of AST expression to storage buffer access
  /// This map has entries added when encountered, and removed when outer
  /// expressions chain the access.
  /// Subset of #expression_order, as expressions are not removed from
  /// #expression_order.
  std::unordered_map<ast::Expression*, StorageBufferAccess> accesses;
  /// The visited order of AST expressions (superset of #accesses)
  std::vector<ast::Expression*> expression_order;
  /// [buffer-type, element-type] -> load function name
  std::unordered_map<TypePair, Symbol, TypePair::Hasher> load_funcs;
  /// [buffer-type, element-type] -> store function name
  std::unordered_map<TypePair, Symbol, TypePair::Hasher> store_funcs;
  /// List of storage buffer writes
  std::vector<Store> stores;

  /// AddAccesss() adds the `expr -> access` map item to #accesses, and `expr`
  /// to #expression_order.
  void AddAccesss(ast::Expression* expr, StorageBufferAccess&& access) {
    accesses.emplace(expr, std::move(access));
    expression_order.emplace_back(expr);
  }

  /// TakeAccess() removes the `node` item from #accesses (if it exists),
  /// returning the StorageBufferAccess. If #accesses does not hold an item for
  /// `node`, an invalid StorageBufferAccess is returned.
  StorageBufferAccess TakeAccess(ast::Expression* node) {
    auto lhs_it = accesses.find(node);
    if (lhs_it == accesses.end()) {
      return {};
    }
    auto access = std::move(lhs_it->second);
    accesses.erase(node);
    return access;
  }

  /// LoadFunc() returns a symbol to an intrinsic function that loads an element
  /// of type `el_ty` from a storage buffer of type `buf_ty`. The function has
  /// the signature: `fn load(buf : buf_ty, offset : u32) -> el_ty`
  Symbol LoadFunc(CloneContext& ctx,
                  Cloneable* insert_after,
                  type::Type* buf_ty,
                  type::Type* el_ty) {
    return utils::GetOrCreate(load_funcs, TypePair{buf_ty, el_ty}, [&] {
      ast::VariableList params = {
          // Note: The buffer parameter requires the kStorage StorageClass in
          // order for HLSL to emit this as a ByteAddressBuffer.
          ctx.dst->create<ast::Variable>(
              ctx.dst->Sym("buffer"), ast::StorageClass::kStorage,
              ctx.Clone(buf_ty), true, nullptr, ast::DecorationList{}),
          ctx.dst->Param("offset", ctx.dst->ty.u32()),
      };

      ast::Function* func = nullptr;
      if (auto* intrinsic = IntrinsicLoadFor(ctx.dst, el_ty)) {
        func = ctx.dst->create<ast::Function>(
            ctx.dst->Symbols().New(), params, ctx.Clone(el_ty), nullptr,
            ast::DecorationList{intrinsic}, ast::DecorationList{});
      } else {
        ast::ExpressionList values;
        if (auto* mat_ty = el_ty->As<type::Matrix>()) {
          auto* vec_ty = ctx.dst->create<type::Vector>(
              ctx.Clone(mat_ty->type()), mat_ty->rows());
          Symbol load = LoadFunc(ctx, insert_after, buf_ty, vec_ty);
          for (uint32_t i = 0; i < mat_ty->columns(); i++) {
            auto* offset =
                ctx.dst->Add("offset", i * MatrixColumnStride(mat_ty));
            values.emplace_back(ctx.dst->Call(load, "buffer", offset));
          }
        } else if (auto* str_ty = el_ty->As<type::Struct>()) {
          auto& sem = ctx.src->Sem();
          auto* str = sem.Get(str_ty);
          for (auto* member : str->Members()) {
            auto* offset = ctx.dst->Add("offset", member->Offset());
            Symbol load = LoadFunc(ctx, insert_after, buf_ty,
                                   member->Declaration()->type()->UnwrapAll());
            values.emplace_back(ctx.dst->Call(load, "buffer", offset));
          }
        } else if (auto* arr_ty = el_ty->As<type::Array>()) {
          auto& sem = ctx.src->Sem();
          auto* arr = sem.Get(arr_ty);
          for (uint32_t i = 0; i < arr_ty->size(); i++) {
            auto* offset = ctx.dst->Add("offset", arr->Stride() * i);
            Symbol load = LoadFunc(ctx, insert_after, buf_ty,
                                   arr_ty->type()->UnwrapAll());
            values.emplace_back(ctx.dst->Call(load, "buffer", offset));
          }
        }
        func = ctx.dst->create<ast::Function>(
            ctx.dst->Symbols().New(), params, ctx.Clone(el_ty),
            ctx.dst->Block(ctx.dst->create<ast::ReturnStatement>(
                ctx.dst->create<ast::TypeConstructorExpression>(
                    ctx.Clone(el_ty), values))),
            ast::DecorationList{}, ast::DecorationList{});
      }
      InsertGlobal(ctx, insert_after, func);
      return func->symbol();
    });
  }

  /// StoreFunc() returns a symbol to an intrinsic function that stores an
  /// element of type `el_ty` to a storage buffer of type `buf_ty`. The function
  /// has the signature: `fn store(buf : buf_ty, offset : u32, value : el_ty)`
  Symbol StoreFunc(CloneContext& ctx,
                   Cloneable* insert_after,
                   type::Type* buf_ty,
                   type::Type* el_ty) {
    return utils::GetOrCreate(store_funcs, TypePair{buf_ty, el_ty}, [&] {
      ast::VariableList params{
          // Note: The buffer parameter requires the kStorage StorageClass in
          // order for HLSL to emit this as a ByteAddressBuffer.
          ctx.dst->create<ast::Variable>(
              ctx.dst->Sym("buffer"), ast::StorageClass::kStorage,
              ctx.Clone(buf_ty), true, nullptr, ast::DecorationList{}),
          ctx.dst->Param("offset", ctx.dst->ty.u32()),
          ctx.dst->Param("value", ctx.Clone(el_ty)),
      };
      ast::Function* func = nullptr;
      if (auto* intrinsic = IntrinsicStoreFor(ctx.dst, el_ty)) {
        func = ctx.dst->create<ast::Function>(
            ctx.dst->Symbols().New(), params, ctx.dst->ty.void_(), nullptr,
            ast::DecorationList{intrinsic}, ast::DecorationList{});

      } else {
        ast::StatementList body;
        if (auto* mat_ty = el_ty->As<type::Matrix>()) {
          auto* vec_ty = ctx.dst->create<type::Vector>(
              ctx.Clone(mat_ty->type()), mat_ty->rows());
          Symbol store = StoreFunc(ctx, insert_after, buf_ty, vec_ty);
          for (uint32_t i = 0; i < mat_ty->columns(); i++) {
            auto* offset =
                ctx.dst->Add("offset", i * MatrixColumnStride(mat_ty));
            auto* access = ctx.dst->IndexAccessor("value", i);
            auto* call = ctx.dst->Call(store, "buffer", offset, access);
            body.emplace_back(ctx.dst->create<ast::CallStatement>(call));
          }
        } else if (auto* str_ty = el_ty->As<type::Struct>()) {
          auto& sem = ctx.src->Sem();
          auto* str = sem.Get(str_ty);
          for (auto* member : str->Members()) {
            auto* offset = ctx.dst->Add("offset", member->Offset());
            auto* access = ctx.dst->MemberAccessor(
                "value", ctx.Clone(member->Declaration()->symbol()));
            Symbol store =
                StoreFunc(ctx, insert_after, buf_ty,
                          member->Declaration()->type()->UnwrapAll());
            auto* call = ctx.dst->Call(store, "buffer", offset, access);
            body.emplace_back(ctx.dst->create<ast::CallStatement>(call));
          }
        } else if (auto* arr_ty = el_ty->As<type::Array>()) {
          auto& sem = ctx.src->Sem();
          auto* arr = sem.Get(arr_ty);
          for (uint32_t i = 0; i < arr_ty->size(); i++) {
            auto* offset = ctx.dst->Add("offset", arr->Stride() * i);
            auto* access = ctx.dst->IndexAccessor("value", ctx.dst->Expr(i));
            Symbol store = StoreFunc(ctx, insert_after, buf_ty,
                                     arr_ty->type()->UnwrapAll());
            auto* call = ctx.dst->Call(store, "buffer", offset, access);
            body.emplace_back(ctx.dst->create<ast::CallStatement>(call));
          }
        }
        func = ctx.dst->create<ast::Function>(
            ctx.dst->Symbols().New(), params, ctx.dst->ty.void_(),
            ctx.dst->Block(body), ast::DecorationList{}, ast::DecorationList{});
      }

      InsertGlobal(ctx, insert_after, func);
      return func->symbol();
    });
  }
};

}  // namespace

DecomposeStorageAccess::Intrinsic::Intrinsic(Type ty) : type(ty) {}
DecomposeStorageAccess::Intrinsic::~Intrinsic() = default;
std::string DecomposeStorageAccess::Intrinsic::Name() const {
  switch (type) {
    case kLoadU32:
      return "intrinsic_load_u32";
    case kLoadF32:
      return "intrinsic_load_f32";
    case kLoadI32:
      return "intrinsic_load_i32";
    case kLoadVec2U32:
      return "intrinsic_load_vec2_u32";
    case kLoadVec2F32:
      return "intrinsic_load_vec2_f32";
    case kLoadVec2I32:
      return "intrinsic_load_vec2_i32";
    case kLoadVec3U32:
      return "intrinsic_load_vec3_u32";
    case kLoadVec3F32:
      return "intrinsic_load_vec3_f32";
    case kLoadVec3I32:
      return "intrinsic_load_vec3_i32";
    case kLoadVec4U32:
      return "intrinsic_load_vec4_u32";
    case kLoadVec4F32:
      return "intrinsic_load_vec4_f32";
    case kLoadVec4I32:
      return "intrinsic_load_vec4_i32";
    case kStoreU32:
      return "intrinsic_store_u32";
    case kStoreF32:
      return "intrinsic_store_f32";
    case kStoreI32:
      return "intrinsic_store_i32";
    case kStoreVec2U32:
      return "intrinsic_store_vec2_u32";
    case kStoreVec2F32:
      return "intrinsic_store_vec2_f32";
    case kStoreVec2I32:
      return "intrinsic_store_vec2_i32";
    case kStoreVec3U32:
      return "intrinsic_store_vec3_u32";
    case kStoreVec3F32:
      return "intrinsic_store_vec3_f32";
    case kStoreVec3I32:
      return "intrinsic_store_vec3_i32";
    case kStoreVec4U32:
      return "intrinsic_store_vec4_u32";
    case kStoreVec4F32:
      return "intrinsic_store_vec4_f32";
    case kStoreVec4I32:
      return "intrinsic_store_vec4_i32";
  }
  return "";
}

DecomposeStorageAccess::Intrinsic* DecomposeStorageAccess::Intrinsic::Clone(
    CloneContext* ctx) const {
  return ctx->dst->ASTNodes().Create<DecomposeStorageAccess::Intrinsic>(type);
}

DecomposeStorageAccess::DecomposeStorageAccess() = default;
DecomposeStorageAccess::~DecomposeStorageAccess() = default;

Transform::Output DecomposeStorageAccess::Run(const Program* in,
                                              const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  auto& sem = ctx.src->Sem();

  State state;

  // Scan the AST nodes for storage buffer accesses. Complex expression chains
  // (e.g. `storage_buffer.foo.bar[20].x`) are handled by maintaining an offset
  // chain via the `state.TakeAccess()`, `state.AddAccess()` methods.
  //
  // Inner-most expression nodes are guaranteed to be visited first because AST
  // nodes are fully immutable and require their children to be constructed
  // first so their pointer can be passed to the parent's constructor.
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* ident = node->As<ast::IdentifierExpression>()) {
      // X
      auto* expr = sem.Get(ident);
      if (auto* var = expr->As<semantic::VariableUser>()) {
        if (var->Variable()->StorageClass() == ast::StorageClass::kStorage) {
          // Variable to a storage buffer
          state.AddAccesss(ident, {
                                      var,
                                      ToOffset(0u),
                                      var->Type()->UnwrapAll(),
                                  });
        }
      }
      continue;
    }

    if (auto* accessor = node->As<ast::MemberAccessorExpression>()) {
      // X.Y
      auto* accessor_sem = sem.Get(accessor);
      auto swizzle = accessor_sem->Swizzle();
      switch (swizzle.size()) {
        case 0: {
          if (auto access = state.TakeAccess(accessor->structure())) {
            auto* str_ty = access.type->As<type::Struct>();
            auto* member =
                sem.Get(str_ty)->FindMember(accessor->member()->symbol());
            auto offset = member->Offset();
            state.AddAccesss(
                accessor, {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              member->Declaration()->type()->UnwrapAll(),
                          });
          }
          break;
        }
        case 1: {
          if (auto access = state.TakeAccess(accessor->structure())) {
            auto* vec_ty = access.type->As<type::Vector>();
            auto offset = Mul(ScalarSize(vec_ty->type()), swizzle[0]);
            state.AddAccesss(
                accessor, {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              vec_ty->type()->UnwrapAll(),
                          });
          }
          break;
        }
      }
      continue;
    }

    if (auto* accessor = node->As<ast::ArrayAccessorExpression>()) {
      if (auto access = state.TakeAccess(accessor->array())) {
        // X[Y]
        if (auto* arr_ty = access.type->As<type::Array>()) {
          auto stride = sem.Get(arr_ty)->Stride();
          auto offset = Mul(stride, accessor->idx_expr());
          state.AddAccesss(accessor,
                           {
                               access.var,
                               Add(std::move(access.offset), std::move(offset)),
                               arr_ty->type()->UnwrapAll(),
                           });
          continue;
        }
        if (auto* vec_ty = access.type->As<type::Vector>()) {
          auto offset = Mul(ScalarSize(vec_ty->type()), accessor->idx_expr());
          state.AddAccesss(accessor,
                           {
                               access.var,
                               Add(std::move(access.offset), std::move(offset)),
                               vec_ty->type()->UnwrapAll(),
                           });
          continue;
        }
        if (auto* mat_ty = access.type->As<type::Matrix>()) {
          auto offset = Mul(MatrixColumnStride(mat_ty), accessor->idx_expr());
          auto* vec_ty = ctx.dst->create<type::Vector>(
              ctx.Clone(mat_ty->type()->UnwrapAll()), mat_ty->rows());
          state.AddAccesss(accessor,
                           {
                               access.var,
                               Add(std::move(access.offset), std::move(offset)),
                               vec_ty,
                           });
          continue;
        }
      }
    }

    if (auto* assign = node->As<ast::AssignmentStatement>()) {
      // X = Y
      // Move the LHS access to a store.
      if (auto lhs = state.TakeAccess(assign->lhs())) {
        state.stores.emplace_back(Store{assign, std::move(lhs)});
      }
    }

    if (auto* call_expr = node->As<ast::CallExpression>()) {
      auto* call = sem.Get(call_expr);
      if (auto* intrinsic = call->Target()->As<semantic::Intrinsic>()) {
        if (intrinsic->Type() == semantic::IntrinsicType::kArrayLength) {
          // arrayLength(X)
          // Don't convert X into a load, this actually requires the real
          // reference.
          state.TakeAccess(call_expr->params()[0]);
        }
      }
    }
  }

  // All remaining accesses are loads, transform these into calls to the
  // corresponding load function
  for (auto* expr : state.expression_order) {
    auto access_it = state.accesses.find(expr);
    if (access_it == state.accesses.end()) {
      continue;
    }

    auto access = std::move(access_it->second);

    auto* buf = access.var->Declaration();
    auto* offset = access.offset->Build(ctx);
    auto* buf_ty = UnwrapPtrAndAlias(access.var->Type());
    auto* el_ty = access.type->UnwrapAll();
    auto* insert_after = ConstructedTypeOf(access.var->Type());
    Symbol func = state.LoadFunc(ctx, insert_after, buf_ty, el_ty);

    auto* load = ctx.dst->Call(func, ctx.Clone(buf), offset);

    ctx.Replace(expr, load);
  }

  // And replace all storage buffer assignments with stores
  for (auto& store : state.stores) {
    auto* buf = store.target.var->Declaration();
    auto* offset = store.target.offset->Build(ctx);
    auto* buf_ty = UnwrapPtrAndAlias(store.target.var->Type());
    auto* el_ty = store.target.type->UnwrapAll();
    auto* value = store.assignment->rhs();
    auto* insert_after = ConstructedTypeOf(store.target.var->Type());
    Symbol func = state.StoreFunc(ctx, insert_after, buf_ty, el_ty);

    auto* call = ctx.dst->Call(func, ctx.Clone(buf), offset, ctx.Clone(value));

    ctx.Replace(store.assignment, ctx.dst->create<ast::CallStatement>(call));
  }

  ctx.Clone();
  return Output{Program(std::move(out))};
}

}  // namespace transform
}  // namespace tint

TINT_INSTANTIATE_TYPEINFO(tint::transform::Offset);
TINT_INSTANTIATE_TYPEINFO(tint::transform::OffsetLiteral);
