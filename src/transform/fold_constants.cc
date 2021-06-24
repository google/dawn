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

#include "src/transform/fold_constants.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/program_builder.h"

namespace tint {

namespace {

using i32 = ProgramBuilder::i32;
using u32 = ProgramBuilder::u32;
using f32 = ProgramBuilder::f32;

/// A Value is a sequence of scalars
struct Value {
  enum class Type {
    i32,  //
    u32,
    f32,
    bool_
  };

  union Scalar {
    ProgramBuilder::i32 i32;
    ProgramBuilder::u32 u32;
    ProgramBuilder::f32 f32;
    bool bool_;

    Scalar(ProgramBuilder::i32 v) : i32(v) {}  // NOLINT
    Scalar(ProgramBuilder::u32 v) : u32(v) {}  // NOLINT
    Scalar(ProgramBuilder::f32 v) : f32(v) {}  // NOLINT
    Scalar(bool v) : bool_(v) {}               // NOLINT
  };

  using Elems = std::vector<Scalar>;

  Type type;
  Elems elems;

  Value() {}

  Value(ProgramBuilder::i32 v) : type(Type::i32), elems{v} {}  // NOLINT
  Value(ProgramBuilder::u32 v) : type(Type::u32), elems{v} {}  // NOLINT
  Value(ProgramBuilder::f32 v) : type(Type::f32), elems{v} {}  // NOLINT
  Value(bool v) : type(Type::bool_), elems{v} {}               // NOLINT

  explicit Value(Type t, Elems e = {}) : type(t), elems(std::move(e)) {}

  bool Valid() const { return elems.size() != 0; }
  operator bool() const { return Valid(); }

  void Append(const Value& value) {
    TINT_ASSERT(Transform, value.type == type);
    elems.insert(elems.end(), value.elems.begin(), value.elems.end());
  }

  /// Calls `func`(s) with s being the current scalar value at `index`.
  /// `func` is typically a lambda of the form '[](auto&& s)'.
  template <typename Func>
  auto WithScalarAt(size_t index, Func&& func) const {
    switch (type) {
      case Value::Type::i32: {
        return func(elems[index].i32);
      }
      case Value::Type::u32: {
        return func(elems[index].u32);
      }
      case Value::Type::f32: {
        return func(elems[index].f32);
      }
      case Value::Type::bool_: {
        return func(elems[index].bool_);
      }
    }
    TINT_ASSERT(Transform, false && "Unreachable");
    return func(~0);
  }
};

/// Returns the Value::Type that maps to the ast::Type*
Value::Type AstToValueType(ast::Type* t) {
  if (t->Is<ast::I32>()) {
    return Value::Type::i32;
  } else if (t->Is<ast::U32>()) {
    return Value::Type::u32;
  } else if (t->Is<ast::F32>()) {
    return Value::Type::f32;
  } else if (t->Is<ast::Bool>()) {
    return Value::Type::bool_;
  }
  TINT_ASSERT(Transform, false && "Invalid type");
  return {};
}

/// Cast `Value` to `target_type`
/// @return the casted value
Value Cast(const Value& value, Value::Type target_type) {
  if (value.type == target_type) {
    return value;
  }

  Value result(target_type);
  for (size_t i = 0; i < value.elems.size(); ++i) {
    switch (target_type) {
      case Value::Type::i32:
        result.Append(value.WithScalarAt(
            i, [](auto&& s) { return static_cast<i32>(s); }));
        break;

      case Value::Type::u32:
        result.Append(value.WithScalarAt(
            i, [](auto&& s) { return static_cast<u32>(s); }));
        break;

      case Value::Type::f32:
        result.Append(value.WithScalarAt(
            i, [](auto&& s) { return static_cast<f32>(s); }));
        break;

      case Value::Type::bool_:
        result.Append(value.WithScalarAt(
            i, [](auto&& s) { return static_cast<bool>(s); }));
        break;
    }
  }

  return result;
}

/// Type that maps `ast::Expression*` to `Value`
using ExprToValue = std::unordered_map<const ast::Expression*, Value>;

/// Adds mapping of `expr` to `value` to `expr_to_value`
/// @returns true if add succeded
bool AddExpr(ExprToValue& expr_to_value,
             const ast::Expression* expr,
             Value value) {
  auto r = expr_to_value.emplace(expr, std::move(value));
  return r.second;
}

/// @returns the `Value` in `expr_to_value` at `expr`, leaving it in the map, or
/// invalid Value if not in map
Value PeekExpr(ExprToValue& expr_to_value, ast::Expression* expr) {
  auto iter = expr_to_value.find(expr);
  if (iter != expr_to_value.end()) {
    return iter->second;
  }
  return {};
}

/// @returns the `Value` in `expr_to_value` at `expr`, removing it from the map,
/// or invalid Value if not in map
Value TakeExpr(ExprToValue& expr_to_value, ast::Expression* expr) {
  auto iter = expr_to_value.find(expr);
  if (iter != expr_to_value.end()) {
    auto result = std::move(iter->second);
    expr_to_value.erase(iter);
    return result;
  }
  return {};
}

/// Folds a `ScalarConstructorExpression` into a `Value`
Value Fold(const ast::ScalarConstructorExpression* scalar_ctor) {
  auto* literal = scalar_ctor->literal();
  if (auto* lit = literal->As<ast::SintLiteral>()) {
    return {lit->value_as_i32()};
  }
  if (auto* lit = literal->As<ast::UintLiteral>()) {
    return {lit->value_as_u32()};
  }
  if (auto* lit = literal->As<ast::FloatLiteral>()) {
    return {lit->value()};
  }
  if (auto* lit = literal->As<ast::BoolLiteral>()) {
    return {lit->IsTrue()};
  }
  TINT_ASSERT(Transform, false && "Unreachable");
  return {};
}

/// Folds a `TypeConstructorExpression` into a `Value` if possible.
/// @returns a valid `Value` with 1 element for scalars, and 2/3/4 elements for
/// vectors.
Value Fold(const ast::TypeConstructorExpression* type_ctor,
           ExprToValue& expr_to_value) {
  auto& ctor_values = type_ctor->values();
  auto* type = type_ctor->type();
  auto* vec = type->As<ast::Vector>();

  // For now, only fold scalars and vectors
  if (!type->is_scalar() && !vec) {
    return {};
  }

  auto* elem_type = vec ? vec->type() : type;
  int result_size = vec ? static_cast<int>(vec->size()) : 1;

  // For zero value init, return 0s
  if (ctor_values.empty()) {
    if (elem_type->Is<ast::I32>()) {
      return Value(Value::Type::i32, Value::Elems(result_size, 0));
    } else if (elem_type->Is<ast::U32>()) {
      return Value(Value::Type::u32, Value::Elems(result_size, 0u));
    } else if (elem_type->Is<ast::F32>()) {
      return Value(Value::Type::f32, Value::Elems(result_size, 0.0f));
    } else if (elem_type->Is<ast::Bool>()) {
      return Value(Value::Type::bool_, Value::Elems(result_size, false));
    }
  }

  // If not all ctor_values are foldable, we can't fold this node
  for (auto* cv : ctor_values) {
    if (!PeekExpr(expr_to_value, cv)) {
      return {};
    }
  }

  // Build value for type_ctor from each child value by casting to
  // type_ctor's type.
  Value new_value(AstToValueType(elem_type));
  for (auto* cv : ctor_values) {
    auto value = TakeExpr(expr_to_value, cv);
    new_value.Append(Cast(value, AstToValueType(elem_type)));
  }

  // Splat single-value initializers
  if (new_value.elems.size() == 1) {
    auto first_value = new_value;
    for (int i = 0; i < result_size - 1; ++i) {
      new_value.Append(first_value);
    }
  }

  return new_value;
}

/// @returns a `ConstructorExpression` to replace `expr` with, or nullptr if we
/// shouldn't replace it.
ast::ConstructorExpression* Build(CloneContext& ctx,
                                  const ast::Expression* expr,
                                  const Value& value) {
  // If original ctor expression had no init values, don't replace the
  // expression
  if (auto* ctor = expr->As<ast::TypeConstructorExpression>()) {
    if (ctor->values().size() == 0) {
      return nullptr;
    }
  }

  auto make_ast_type = [&]() -> ast::Type* {
    switch (value.type) {
      case Value::Type::i32:
        return ctx.dst->ty.i32();
      case Value::Type::u32:
        return ctx.dst->ty.u32();
      case Value::Type::f32:
        return ctx.dst->ty.f32();
      case Value::Type::bool_:
        return ctx.dst->ty.bool_();
    }
    return nullptr;
  };

  if (auto* type_ctor = expr->As<ast::TypeConstructorExpression>()) {
    if (auto* vec = type_ctor->type()->As<ast::Vector>()) {
      uint32_t vec_size = static_cast<uint32_t>(vec->size());

      // We'd like to construct the new vector with the same number of
      // constructor args that the original node had, but after folding
      // constants, cases like the following are problematic:
      //
      // vec3<f32> = vec3<f32>(vec2<f32>, 1.0) // vec_size=3, ctor_size=2
      //
      // In this case, creating a vec3 with 2 args is invalid, so we should
      // create it with 3. So what we do is construct with vec_size args,
      // except if the original vector was single-value initialized, in which
      // case, we only construct with one arg again.
      uint32_t ctor_size = (type_ctor->values().size() == 1) ? 1 : vec_size;

      ast::ExpressionList ctors;
      for (uint32_t i = 0; i < ctor_size; ++i) {
        value.WithScalarAt(
            i, [&](auto&& s) { ctors.emplace_back(ctx.dst->Expr(s)); });
      }

      return ctx.dst->vec(make_ast_type(), vec_size, ctors);
    } else if (type_ctor->type()->is_scalar()) {
      return value.WithScalarAt(0, [&](auto&& s) { return ctx.dst->Expr(s); });
    }
  }
  return nullptr;
}

}  // namespace

namespace transform {

FoldConstants::FoldConstants() = default;

FoldConstants::~FoldConstants() = default;

Output FoldConstants::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  ExprToValue expr_to_value;

  // Visit inner expressions before outer expressions
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* scalar_ctor = node->As<ast::ScalarConstructorExpression>()) {
      if (auto v = Fold(scalar_ctor)) {
        AddExpr(expr_to_value, scalar_ctor, std::move(v));
      }
    }
    if (auto* type_ctor = node->As<ast::TypeConstructorExpression>()) {
      if (auto v = Fold(type_ctor, expr_to_value)) {
        AddExpr(expr_to_value, type_ctor, std::move(v));
      }
    }
  }

  for (auto& kvp : expr_to_value) {
    if (auto* ctor_expr = Build(ctx, kvp.first, kvp.second)) {
      ctx.Replace(kvp.first, ctor_expr);
    }
  }

  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
