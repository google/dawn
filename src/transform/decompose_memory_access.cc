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

#include "src/transform/decompose_memory_access.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/assignment_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/disable_validation_decoration.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type_name.h"
#include "src/ast/unary_op.h"
#include "src/program_builder.h"
#include "src/sem/array.h"
#include "src/sem/atomic_type.h"
#include "src/sem/call.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/reference_type.h"
#include "src/sem/statement.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"
#include "src/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::DecomposeMemoryAccess);
TINT_INSTANTIATE_TYPEINFO(tint::transform::DecomposeMemoryAccess::Intrinsic);

namespace tint {
namespace transform {

namespace {

/// Offset is a simple ast::Expression builder interface, used to build byte
/// offsets for storage and uniform buffer accesses.
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
    auto* type = ctx.src->Sem().Get(expr)->Type()->UnwrapRef();
    auto* res = ctx.Clone(expr);
    if (!type->Is<sem::U32>()) {
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

/// LoadStoreKey is the unordered map key to a load or store intrinsic.
struct LoadStoreKey {
  ast::StorageClass const storage_class;  // buffer storage class
  sem::Type const* buf_ty;                // buffer type
  sem::Type const* el_ty;                 // element type
  bool operator==(const LoadStoreKey& rhs) const {
    return storage_class == rhs.storage_class && buf_ty == rhs.buf_ty &&
           el_ty == rhs.el_ty;
  }
  struct Hasher {
    inline std::size_t operator()(const LoadStoreKey& u) const {
      return utils::Hash(u.storage_class, u.buf_ty, u.el_ty);
    }
  };
};

/// AtomicKey is the unordered map key to an atomic intrinsic.
struct AtomicKey {
  sem::Type const* buf_ty;      // buffer type
  sem::Type const* el_ty;       // element type
  sem::IntrinsicType const op;  // atomic op
  bool operator==(const AtomicKey& rhs) const {
    return buf_ty == rhs.buf_ty && el_ty == rhs.el_ty && op == rhs.op;
  }
  struct Hasher {
    inline std::size_t operator()(const AtomicKey& u) const {
      return utils::Hash(u.buf_ty, u.el_ty, u.op);
    }
  };
};

/// @returns the size in bytes of a scalar
uint32_t ScalarSize(const sem::Type*) {
  // TODO(bclayton): Assumes 32-bit elements
  return 4;
}

/// @returns the number of bytes between columns of the given matrix
uint32_t MatrixColumnStride(const sem::Matrix* mat) {
  return ScalarSize(mat->type()) * ((mat->rows() == 2) ? 2 : 4);
}

bool IntrinsicDataTypeFor(const sem::Type* ty,
                          DecomposeMemoryAccess::Intrinsic::DataType& out) {
  if (ty->Is<sem::I32>()) {
    out = DecomposeMemoryAccess::Intrinsic::DataType::kI32;
    return true;
  }
  if (ty->Is<sem::U32>()) {
    out = DecomposeMemoryAccess::Intrinsic::DataType::kU32;
    return true;
  }
  if (ty->Is<sem::F32>()) {
    out = DecomposeMemoryAccess::Intrinsic::DataType::kF32;
    return true;
  }
  if (auto* vec = ty->As<sem::Vector>()) {
    switch (vec->size()) {
      case 2:
        if (vec->type()->Is<sem::I32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2I32;
          return true;
        }
        if (vec->type()->Is<sem::U32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2U32;
          return true;
        }
        if (vec->type()->Is<sem::F32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2F32;
          return true;
        }
        break;
      case 3:
        if (vec->type()->Is<sem::I32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3I32;
          return true;
        }
        if (vec->type()->Is<sem::U32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3U32;
          return true;
        }
        if (vec->type()->Is<sem::F32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3F32;
          return true;
        }
        break;
      case 4:
        if (vec->type()->Is<sem::I32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4I32;
          return true;
        }
        if (vec->type()->Is<sem::U32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4U32;
          return true;
        }
        if (vec->type()->Is<sem::F32>()) {
          out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4F32;
          return true;
        }
        break;
    }
    return false;
  }

  return false;
}

/// @returns a DecomposeMemoryAccess::Intrinsic decoration that can be applied
/// to a stub function to load the type `ty`.
DecomposeMemoryAccess::Intrinsic* IntrinsicLoadFor(
    ProgramBuilder* builder,
    ast::StorageClass storage_class,
    const sem::Type* ty) {
  DecomposeMemoryAccess::Intrinsic::DataType type;
  if (!IntrinsicDataTypeFor(ty, type)) {
    return nullptr;
  }
  return builder->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
      builder->ID(), DecomposeMemoryAccess::Intrinsic::Op::kLoad, storage_class,
      type);
}

/// @returns a DecomposeMemoryAccess::Intrinsic decoration that can be applied
/// to a stub function to store the type `ty`.
DecomposeMemoryAccess::Intrinsic* IntrinsicStoreFor(
    ProgramBuilder* builder,
    ast::StorageClass storage_class,
    const sem::Type* ty) {
  DecomposeMemoryAccess::Intrinsic::DataType type;
  if (!IntrinsicDataTypeFor(ty, type)) {
    return nullptr;
  }
  return builder->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
      builder->ID(), DecomposeMemoryAccess::Intrinsic::Op::kStore,
      storage_class, type);
}

/// @returns a DecomposeMemoryAccess::Intrinsic decoration that can be applied
/// to a stub function for the atomic op and the type `ty`.
DecomposeMemoryAccess::Intrinsic* IntrinsicAtomicFor(ProgramBuilder* builder,
                                                     sem::IntrinsicType ity,
                                                     const sem::Type* ty) {
  auto op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicLoad;
  switch (ity) {
    case sem::IntrinsicType::kAtomicLoad:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicLoad;
      break;
    case sem::IntrinsicType::kAtomicStore:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicStore;
      break;
    case sem::IntrinsicType::kAtomicAdd:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicAdd;
      break;
    case sem::IntrinsicType::kAtomicMax:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicMax;
      break;
    case sem::IntrinsicType::kAtomicMin:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicMin;
      break;
    case sem::IntrinsicType::kAtomicAnd:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicAnd;
      break;
    case sem::IntrinsicType::kAtomicOr:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicOr;
      break;
    case sem::IntrinsicType::kAtomicXor:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicXor;
      break;
    case sem::IntrinsicType::kAtomicExchange:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicExchange;
      break;
    case sem::IntrinsicType::kAtomicCompareExchangeWeak:
      op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicCompareExchangeWeak;
      break;
    default:
      TINT_ICE(Transform, builder->Diagnostics())
          << "invalid IntrinsicType for DecomposeMemoryAccess::Intrinsic: "
          << ty->type_name();
      break;
  }

  DecomposeMemoryAccess::Intrinsic::DataType type;
  if (!IntrinsicDataTypeFor(ty, type)) {
    return nullptr;
  }
  return builder->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
      builder->ID(), op, ast::StorageClass::kStorage, type);
}

/// Inserts `node` before `insert_after` in the global declarations of
/// `ctx.dst`. If `insert_after` is nullptr, then `node` is inserted at the top
/// of the module.
void InsertGlobal(CloneContext& ctx,
                  const Cloneable* insert_after,
                  Cloneable* node) {
  auto& globals = ctx.src->AST().GlobalDeclarations();
  if (insert_after) {
    ctx.InsertAfter(globals, insert_after, node);
  } else {
    ctx.InsertBefore(globals, *globals.begin(), node);
  }
}

/// @returns the unwrapped, user-declared type of ty.
const ast::TypeDecl* TypeDeclOf(const sem::Type* ty) {
  while (true) {
    if (auto* ref = ty->As<sem::Reference>()) {
      ty = ref->StoreType();
      continue;
    }
    if (auto* str = ty->As<sem::Struct>()) {
      return str->Declaration();
    }
    // Not a declared type
    return nullptr;
  }
}

/// BufferAccess describes a single storage or uniform buffer access
struct BufferAccess {
  sem::Expression const* var = nullptr;  // Storage buffer variable
  std::unique_ptr<Offset> offset;        // The byte offset on var
  sem::Type const* type = nullptr;       // The type of the access
  operator bool() const { return var; }  // Returns true if valid
};

/// Store describes a single storage or uniform buffer write
struct Store {
  ast::AssignmentStatement* assignment;  // The AST assignment statement
  BufferAccess target;                   // The target for the write
};

}  // namespace

/// State holds the current transform state
struct DecomposeMemoryAccess::State {
  /// Map of AST expression to storage or uniform buffer access
  /// This map has entries added when encountered, and removed when outer
  /// expressions chain the access.
  /// Subset of #expression_order, as expressions are not removed from
  /// #expression_order.
  std::unordered_map<ast::Expression*, BufferAccess> accesses;
  /// The visited order of AST expressions (superset of #accesses)
  std::vector<ast::Expression*> expression_order;
  /// [buffer-type, element-type] -> load function name
  std::unordered_map<LoadStoreKey, Symbol, LoadStoreKey::Hasher> load_funcs;
  /// [buffer-type, element-type] -> store function name
  std::unordered_map<LoadStoreKey, Symbol, LoadStoreKey::Hasher> store_funcs;
  /// [buffer-type, element-type, atomic-op] -> load function name
  std::unordered_map<AtomicKey, Symbol, AtomicKey::Hasher> atomic_funcs;
  /// List of storage or uniform buffer writes
  std::vector<Store> stores;

  /// AddAccess() adds the `expr -> access` map item to #accesses, and `expr`
  /// to #expression_order.
  /// @param expr the expression that performs the access
  /// @param access the access
  void AddAccess(ast::Expression* expr, BufferAccess&& access) {
    TINT_ASSERT(Transform, access.type);
    accesses.emplace(expr, std::move(access));
    expression_order.emplace_back(expr);
  }

  /// TakeAccess() removes the `node` item from #accesses (if it exists),
  /// returning the BufferAccess. If #accesses does not hold an item for
  /// `node`, an invalid BufferAccess is returned.
  /// @param node the expression that performed an access
  /// @return the BufferAccess for the given expression
  BufferAccess TakeAccess(ast::Expression* node) {
    auto lhs_it = accesses.find(node);
    if (lhs_it == accesses.end()) {
      return {};
    }
    auto access = std::move(lhs_it->second);
    accesses.erase(node);
    return access;
  }

  /// LoadFunc() returns a symbol to an intrinsic function that loads an element
  /// of type `el_ty` from a storage or uniform buffer of type `buf_ty`.
  /// The emitted function has the signature:
  ///   `fn load(buf : buf_ty, offset : u32) -> el_ty`
  /// @param ctx the CloneContext
  /// @param insert_after the user-declared type to insert the function after
  /// @param buf_ty the storage or uniform buffer type
  /// @param el_ty the storage or uniform buffer element type
  /// @param var_user the variable user
  /// @return the name of the function that performs the load
  Symbol LoadFunc(CloneContext& ctx,
                  const ast::TypeDecl* insert_after,
                  const sem::Type* buf_ty,
                  const sem::Type* el_ty,
                  const sem::VariableUser* var_user) {
    auto storage_class = var_user->Variable()->StorageClass();
    return utils::GetOrCreate(
        load_funcs, LoadStoreKey{storage_class, buf_ty, el_ty}, [&] {
          auto* buf_ast_ty = CreateASTTypeFor(&ctx, buf_ty);
          auto* disable_validation =
              ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                  ctx.dst->ID(), ast::DisabledValidation::
                                     kIgnoreConstructibleFunctionParameter);

          ast::VariableList params = {
              // Note: The buffer parameter requires the StorageClass in
              // order for HLSL to emit this as a ByteAddressBuffer or cbuffer
              // array.
              ctx.dst->create<ast::Variable>(
                  ctx.dst->Sym("buffer"), storage_class,
                  var_user->Variable()->Access(), buf_ast_ty, true, nullptr,
                  ast::DecorationList{disable_validation}),
              ctx.dst->Param("offset", ctx.dst->ty.u32()),
          };

          ast::Function* func = nullptr;
          if (auto* intrinsic =
                  IntrinsicLoadFor(ctx.dst, storage_class, el_ty)) {
            auto* el_ast_ty = CreateASTTypeFor(&ctx, el_ty);
            func = ctx.dst->create<ast::Function>(
                ctx.dst->Sym(), params, el_ast_ty, nullptr,
                ast::DecorationList{
                    intrinsic,
                    ctx.dst->ASTNodes()
                        .Create<ast::DisableValidationDecoration>(
                            ctx.dst->ID(),
                            ast::DisabledValidation::kFunctionHasNoBody),
                },
                ast::DecorationList{});
          } else {
            ast::ExpressionList values;
            if (auto* mat_ty = el_ty->As<sem::Matrix>()) {
              auto* vec_ty = mat_ty->ColumnType();
              Symbol load =
                  LoadFunc(ctx, insert_after, buf_ty, vec_ty, var_user);
              for (uint32_t i = 0; i < mat_ty->columns(); i++) {
                auto* offset =
                    ctx.dst->Add("offset", i * MatrixColumnStride(mat_ty));
                values.emplace_back(ctx.dst->Call(load, "buffer", offset));
              }
            } else if (auto* str = el_ty->As<sem::Struct>()) {
              for (auto* member : str->Members()) {
                auto* offset = ctx.dst->Add("offset", member->Offset());
                Symbol load = LoadFunc(ctx, insert_after, buf_ty,
                                       member->Type()->UnwrapRef(), var_user);
                values.emplace_back(ctx.dst->Call(load, "buffer", offset));
              }
            } else if (auto* arr = el_ty->As<sem::Array>()) {
              for (uint32_t i = 0; i < arr->Count(); i++) {
                auto* offset = ctx.dst->Add("offset", arr->Stride() * i);
                Symbol load = LoadFunc(ctx, insert_after, buf_ty,
                                       arr->ElemType()->UnwrapRef(), var_user);
                values.emplace_back(ctx.dst->Call(load, "buffer", offset));
              }
            }
            auto* el_ast_ty = CreateASTTypeFor(&ctx, el_ty);
            func = ctx.dst->create<ast::Function>(
                ctx.dst->Sym(), params, el_ast_ty,
                ctx.dst->Block(ctx.dst->Return(
                    ctx.dst->create<ast::TypeConstructorExpression>(
                        CreateASTTypeFor(&ctx, el_ty), values))),
                ast::DecorationList{}, ast::DecorationList{});
          }
          InsertGlobal(ctx, insert_after, func);
          return func->symbol();
        });
  }

  /// StoreFunc() returns a symbol to an intrinsic function that stores an
  /// element of type `el_ty` to a storage buffer of type `buf_ty`.
  /// The function has the signature:
  ///   `fn store(buf : buf_ty, offset : u32, value : el_ty)`
  /// @param ctx the CloneContext
  /// @param insert_after the user-declared type to insert the function after
  /// @param buf_ty the storage buffer type
  /// @param el_ty the storage buffer element type
  /// @param var_user the variable user
  /// @return the name of the function that performs the store
  Symbol StoreFunc(CloneContext& ctx,
                   const ast::TypeDecl* insert_after,
                   const sem::Type* buf_ty,
                   const sem::Type* el_ty,
                   const sem::VariableUser* var_user) {
    auto storage_class = var_user->Variable()->StorageClass();
    return utils::GetOrCreate(
        store_funcs, LoadStoreKey{storage_class, buf_ty, el_ty}, [&] {
          auto* buf_ast_ty = CreateASTTypeFor(&ctx, buf_ty);
          auto* el_ast_ty = CreateASTTypeFor(&ctx, el_ty);
          auto* disable_validation =
              ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                  ctx.dst->ID(), ast::DisabledValidation::
                                     kIgnoreConstructibleFunctionParameter);
          ast::VariableList params{
              // Note: The buffer parameter requires the StorageClass in
              // order for HLSL to emit this as a ByteAddressBuffer.

              ctx.dst->create<ast::Variable>(
                  ctx.dst->Sym("buffer"), storage_class,
                  var_user->Variable()->Access(), buf_ast_ty, true, nullptr,
                  ast::DecorationList{disable_validation}),
              ctx.dst->Param("offset", ctx.dst->ty.u32()),
              ctx.dst->Param("value", el_ast_ty),
          };
          ast::Function* func = nullptr;
          if (auto* intrinsic =
                  IntrinsicStoreFor(ctx.dst, storage_class, el_ty)) {
            func = ctx.dst->create<ast::Function>(
                ctx.dst->Sym(), params, ctx.dst->ty.void_(), nullptr,
                ast::DecorationList{
                    intrinsic,
                    ctx.dst->ASTNodes()
                        .Create<ast::DisableValidationDecoration>(
                            ctx.dst->ID(),
                            ast::DisabledValidation::kFunctionHasNoBody),
                },
                ast::DecorationList{});

          } else {
            ast::StatementList body;
            if (auto* mat_ty = el_ty->As<sem::Matrix>()) {
              auto* vec_ty = mat_ty->ColumnType();
              Symbol store =
                  StoreFunc(ctx, insert_after, buf_ty, vec_ty, var_user);
              for (uint32_t i = 0; i < mat_ty->columns(); i++) {
                auto* offset =
                    ctx.dst->Add("offset", i * MatrixColumnStride(mat_ty));
                auto* access = ctx.dst->IndexAccessor("value", i);
                auto* call = ctx.dst->Call(store, "buffer", offset, access);
                body.emplace_back(ctx.dst->create<ast::CallStatement>(call));
              }
            } else if (auto* str = el_ty->As<sem::Struct>()) {
              for (auto* member : str->Members()) {
                auto* offset = ctx.dst->Add("offset", member->Offset());
                auto* access = ctx.dst->MemberAccessor(
                    "value", ctx.Clone(member->Declaration()->symbol()));
                Symbol store = StoreFunc(ctx, insert_after, buf_ty,
                                         member->Type()->UnwrapRef(), var_user);
                auto* call = ctx.dst->Call(store, "buffer", offset, access);
                body.emplace_back(ctx.dst->create<ast::CallStatement>(call));
              }
            } else if (auto* arr = el_ty->As<sem::Array>()) {
              for (uint32_t i = 0; i < arr->Count(); i++) {
                auto* offset = ctx.dst->Add("offset", arr->Stride() * i);
                auto* access =
                    ctx.dst->IndexAccessor("value", ctx.dst->Expr(i));
                Symbol store =
                    StoreFunc(ctx, insert_after, buf_ty,
                              arr->ElemType()->UnwrapRef(), var_user);
                auto* call = ctx.dst->Call(store, "buffer", offset, access);
                body.emplace_back(ctx.dst->create<ast::CallStatement>(call));
              }
            }
            func = ctx.dst->create<ast::Function>(
                ctx.dst->Sym(), params, ctx.dst->ty.void_(),
                ctx.dst->Block(body), ast::DecorationList{},
                ast::DecorationList{});
          }

          InsertGlobal(ctx, insert_after, func);
          return func->symbol();
        });
  }

  /// AtomicFunc() returns a symbol to an intrinsic function that performs an
  /// atomic operation from a storage buffer of type `buf_ty`. The function has
  /// the signature:
  // `fn atomic_op(buf : buf_ty, offset : u32, ...) -> T`
  /// @param ctx the CloneContext
  /// @param insert_after the user-declared type to insert the function after
  /// @param buf_ty the storage buffer type
  /// @param el_ty the storage buffer element type
  /// @param intrinsic the atomic intrinsic
  /// @param var_user the variable user
  /// @return the name of the function that performs the load
  Symbol AtomicFunc(CloneContext& ctx,
                    const ast::TypeDecl* insert_after,
                    const sem::Type* buf_ty,
                    const sem::Type* el_ty,
                    const sem::Intrinsic* intrinsic,
                    const sem::VariableUser* var_user) {
    auto op = intrinsic->Type();
    return utils::GetOrCreate(atomic_funcs, AtomicKey{buf_ty, el_ty, op}, [&] {
      auto* buf_ast_ty = CreateASTTypeFor(&ctx, buf_ty);
      auto* disable_validation =
          ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
              ctx.dst->ID(),
              ast::DisabledValidation::kIgnoreConstructibleFunctionParameter);
      // The first parameter to all WGSL atomics is the expression to the
      // atomic. This is replaced with two parameters: the buffer and offset.

      ast::VariableList params = {
          // Note: The buffer parameter requires the kStorage StorageClass in
          // order for HLSL to emit this as a ByteAddressBuffer.
          ctx.dst->create<ast::Variable>(
              ctx.dst->Sym("buffer"), ast::StorageClass::kStorage,
              var_user->Variable()->Access(), buf_ast_ty, true, nullptr,
              ast::DecorationList{disable_validation}),
          ctx.dst->Param("offset", ctx.dst->ty.u32()),
      };

      // Other parameters are copied as-is:
      for (size_t i = 1; i < intrinsic->Parameters().size(); i++) {
        auto& param = intrinsic->Parameters()[i];
        auto* ty = CreateASTTypeFor(&ctx, param.type);
        params.emplace_back(ctx.dst->Param("param_" + std::to_string(i), ty));
      }

      auto* atomic = IntrinsicAtomicFor(ctx.dst, op, el_ty);
      if (atomic == nullptr) {
        TINT_ICE(Transform, ctx.dst->Diagnostics())
            << "IntrinsicAtomicFor() returned nullptr for op " << op
            << " and type " << el_ty->type_name();
      }

      auto* ret_ty = CreateASTTypeFor(&ctx, intrinsic->ReturnType());
      auto* func = ctx.dst->create<ast::Function>(
          ctx.dst->Sym(), params, ret_ty, nullptr,
          ast::DecorationList{
              atomic,
              ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                  ctx.dst->ID(), ast::DisabledValidation::kFunctionHasNoBody),
          },
          ast::DecorationList{});

      InsertGlobal(ctx, insert_after, func);
      return func->symbol();
    });
  }
};

DecomposeMemoryAccess::Intrinsic::Intrinsic(ProgramID program_id,
                                            Op o,
                                            ast::StorageClass sc,
                                            DataType ty)
    : Base(program_id), op(o), storage_class(sc), type(ty) {}
DecomposeMemoryAccess::Intrinsic::~Intrinsic() = default;
std::string DecomposeMemoryAccess::Intrinsic::InternalName() const {
  std::stringstream ss;
  switch (op) {
    case Op::kLoad:
      ss << "intrinsic_load_";
      break;
    case Op::kStore:
      ss << "intrinsic_store_";
      break;
    case Op::kAtomicLoad:
      ss << "intrinsic_atomic_load_";
      break;
    case Op::kAtomicStore:
      ss << "intrinsic_atomic_store_";
      break;
    case Op::kAtomicAdd:
      ss << "intrinsic_atomic_add_";
      break;
    case Op::kAtomicMax:
      ss << "intrinsic_atomic_max_";
      break;
    case Op::kAtomicMin:
      ss << "intrinsic_atomic_min_";
      break;
    case Op::kAtomicAnd:
      ss << "intrinsic_atomic_and_";
      break;
    case Op::kAtomicOr:
      ss << "intrinsic_atomic_or_";
      break;
    case Op::kAtomicXor:
      ss << "intrinsic_atomic_xor_";
      break;
    case Op::kAtomicExchange:
      ss << "intrinsic_atomic_exchange_";
      break;
    case Op::kAtomicCompareExchangeWeak:
      ss << "intrinsic_atomic_compare_exchange_weak_";
      break;
  }
  ss << storage_class << "_";
  switch (type) {
    case DataType::kU32:
      ss << "u32";
      break;
    case DataType::kF32:
      ss << "f32";
      break;
    case DataType::kI32:
      ss << "i32";
      break;
    case DataType::kVec2U32:
      ss << "vec2_u32";
      break;
    case DataType::kVec2F32:
      ss << "vec2_f32";
      break;
    case DataType::kVec2I32:
      ss << "vec2_i32";
      break;
    case DataType::kVec3U32:
      ss << "vec3_u32";
      break;
    case DataType::kVec3F32:
      ss << "vec3_f32";
      break;
    case DataType::kVec3I32:
      ss << "vec3_i32";
      break;
    case DataType::kVec4U32:
      ss << "vec4_u32";
      break;
    case DataType::kVec4F32:
      ss << "vec4_f32";
      break;
    case DataType::kVec4I32:
      ss << "vec4_i32";
      break;
  }
  return ss.str();
}

DecomposeMemoryAccess::Intrinsic* DecomposeMemoryAccess::Intrinsic::Clone(
    CloneContext* ctx) const {
  return ctx->dst->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
      ctx->dst->ID(), op, storage_class, type);
}

DecomposeMemoryAccess::DecomposeMemoryAccess() = default;
DecomposeMemoryAccess::~DecomposeMemoryAccess() = default;

void DecomposeMemoryAccess::Run(CloneContext& ctx, const DataMap&, DataMap&) {
  auto& sem = ctx.src->Sem();

  State state;

  // Scan the AST nodes for storage and uniform buffer accesses. Complex
  // expression chains (e.g. `storage_buffer.foo.bar[20].x`) are handled by
  // maintaining an offset chain via the `state.TakeAccess()`,
  // `state.AddAccess()` methods.
  //
  // Inner-most expression nodes are guaranteed to be visited first because AST
  // nodes are fully immutable and require their children to be constructed
  // first so their pointer can be passed to the parent's constructor.
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* ident = node->As<ast::IdentifierExpression>()) {
      // X
      if (auto* var = sem.Get<sem::VariableUser>(ident)) {
        if (var->Variable()->StorageClass() == ast::StorageClass::kStorage ||
            var->Variable()->StorageClass() == ast::StorageClass::kUniform) {
          // Variable to a storage or uniform buffer
          state.AddAccess(ident, {
                                     var,
                                     ToOffset(0u),
                                     var->Type()->UnwrapRef(),
                                 });
        }
      }
      continue;
    }

    if (auto* accessor = node->As<ast::MemberAccessorExpression>()) {
      // X.Y
      auto* accessor_sem = sem.Get(accessor);
      if (auto* swizzle = accessor_sem->As<sem::Swizzle>()) {
        if (swizzle->Indices().size() == 1) {
          if (auto access = state.TakeAccess(accessor->structure())) {
            auto* vec_ty = access.type->As<sem::Vector>();
            auto offset =
                Mul(ScalarSize(vec_ty->type()), swizzle->Indices()[0]);
            state.AddAccess(
                accessor, {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              vec_ty->type()->UnwrapRef(),
                          });
          }
        }
      } else {
        if (auto access = state.TakeAccess(accessor->structure())) {
          auto* str_ty = access.type->As<sem::Struct>();
          auto* member = str_ty->FindMember(accessor->member()->symbol());
          auto offset = member->Offset();
          state.AddAccess(accessor,
                          {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              member->Type()->UnwrapRef(),
                          });
        }
      }
      continue;
    }

    if (auto* accessor = node->As<ast::ArrayAccessorExpression>()) {
      if (auto access = state.TakeAccess(accessor->array())) {
        // X[Y]
        if (auto* arr = access.type->As<sem::Array>()) {
          auto offset = Mul(arr->Stride(), accessor->idx_expr());
          state.AddAccess(accessor,
                          {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              arr->ElemType()->UnwrapRef(),
                          });
          continue;
        }
        if (auto* vec_ty = access.type->As<sem::Vector>()) {
          auto offset = Mul(ScalarSize(vec_ty->type()), accessor->idx_expr());
          state.AddAccess(accessor,
                          {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              vec_ty->type()->UnwrapRef(),
                          });
          continue;
        }
        if (auto* mat_ty = access.type->As<sem::Matrix>()) {
          auto offset = Mul(MatrixColumnStride(mat_ty), accessor->idx_expr());
          state.AddAccess(accessor,
                          {
                              access.var,
                              Add(std::move(access.offset), std::move(offset)),
                              mat_ty->ColumnType(),
                          });
          continue;
        }
      }
    }

    if (auto* op = node->As<ast::UnaryOpExpression>()) {
      if (op->op() == ast::UnaryOp::kAddressOf) {
        // &X
        if (auto access = state.TakeAccess(op->expr())) {
          // HLSL does not support pointers, so just take the access from the
          // reference and place it on the pointer.
          state.AddAccess(op, std::move(access));
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
      if (auto* intrinsic = call->Target()->As<sem::Intrinsic>()) {
        if (intrinsic->Type() == sem::IntrinsicType::kArrayLength) {
          // arrayLength(X)
          // Don't convert X into a load, this intrinsic actually requires the
          // real pointer.
          state.TakeAccess(call_expr->params()[0]);
        }
        if (intrinsic->IsAtomic()) {
          if (auto access = state.TakeAccess(call_expr->params()[0])) {
            // atomic___(X)

            auto* buf = access.var->Declaration();
            auto* offset = access.offset->Build(ctx);
            auto* buf_ty = access.var->Type()->UnwrapRef();
            auto* el_ty = access.type->UnwrapRef()->As<sem::Atomic>()->Type();
            auto* insert_after = TypeDeclOf(access.var->Type());
            Symbol func =
                state.AtomicFunc(ctx, insert_after, buf_ty, el_ty, intrinsic,
                                 access.var->As<sem::VariableUser>());

            ast::ExpressionList args{ctx.Clone(buf), offset};
            for (size_t i = 1; i < call_expr->params().size(); i++) {
              auto* arg = call_expr->params()[i];
              args.emplace_back(ctx.Clone(arg));
            }

            ctx.Replace(call_expr, ctx.dst->Call(func, args));
          }
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
    auto* buf_ty = access.var->Type()->UnwrapRef();
    auto* el_ty = access.type->UnwrapRef();
    auto* insert_after = TypeDeclOf(access.var->Type());
    Symbol func = state.LoadFunc(ctx, insert_after, buf_ty, el_ty,
                                 access.var->As<sem::VariableUser>());

    auto* load = ctx.dst->Call(func, ctx.Clone(buf), offset);

    ctx.Replace(expr, load);
  }

  // And replace all storage and uniform buffer assignments with stores
  for (auto& store : state.stores) {
    auto* buf = store.target.var->Declaration();
    auto* offset = store.target.offset->Build(ctx);
    auto* buf_ty = store.target.var->Type()->UnwrapRef();
    auto* el_ty = store.target.type->UnwrapRef();
    auto* value = store.assignment->rhs();
    auto* insert_after = TypeDeclOf(store.target.var->Type());
    Symbol func = state.StoreFunc(ctx, insert_after, buf_ty, el_ty,
                                  store.target.var->As<sem::VariableUser>());

    auto* call = ctx.dst->Call(func, ctx.Clone(buf), offset, ctx.Clone(value));

    ctx.Replace(store.assignment, ctx.dst->create<ast::CallStatement>(call));
  }

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint

TINT_INSTANTIATE_TYPEINFO(tint::transform::Offset);
TINT_INSTANTIATE_TYPEINFO(tint::transform::OffsetLiteral);
