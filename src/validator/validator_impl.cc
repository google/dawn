// Copyright 2020 The Tint Authors.
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

#include "src/validator/validator_impl.h"

#include <cassert>
#include <unordered_set>
#include <utility>

#include "src/ast/call_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/function.h"
#include "src/ast/int_literal.h"
#include "src/ast/intrinsic.h"
#include "src/ast/module.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/switch_statement.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable_decl_statement.h"
#include "src/semantic/expression.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"

namespace tint {

namespace {

enum class IntrinsicDataType {
  kMixed,
  kFloatOrIntScalarOrVector,
  kFloatScalarOrVector,
  kIntScalarOrVector,
  kFloatVector,
  kFloatScalar,
  kMatrix,
};

struct IntrinsicData {
  ast::Intrinsic intrinsic;
  uint32_t param_count;
  IntrinsicDataType data_type;
  uint32_t vector_size;
  bool all_types_match;
};

// Note, this isn't all the intrinsics. Some are handled specially before
// we get to the generic code. See the ValidateCallExpr code below.
constexpr const IntrinsicData kIntrinsicData[] = {
    {ast::Intrinsic::kAbs, 1, IntrinsicDataType::kFloatOrIntScalarOrVector, 0,
     true},
    {ast::Intrinsic::kAcos, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kAsin, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kAtan, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kAtan2, 2, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kCeil, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kClamp, 3, IntrinsicDataType::kFloatOrIntScalarOrVector, 0,
     true},
    {ast::Intrinsic::kCos, 1, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kCosh, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kCountOneBits, 1, IntrinsicDataType::kIntScalarOrVector, 0,
     true},
    {ast::Intrinsic::kCross, 2, IntrinsicDataType::kFloatVector, 3, true},
    {ast::Intrinsic::kDeterminant, 1, IntrinsicDataType::kMatrix, 0, false},
    {ast::Intrinsic::kDistance, 2, IntrinsicDataType::kFloatScalarOrVector, 0,
     false},
    {ast::Intrinsic::kExp, 1, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kExp2, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kFaceForward, 3, IntrinsicDataType::kFloatScalarOrVector,
     0, true},
    {ast::Intrinsic::kFloor, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kFma, 3, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kFract, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kFrexp, 2, IntrinsicDataType::kMixed, 0, false},
    {ast::Intrinsic::kInverseSqrt, 1, IntrinsicDataType::kFloatScalarOrVector,
     0, true},
    {ast::Intrinsic::kLdexp, 2, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kLength, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     false},
    {ast::Intrinsic::kLog, 1, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kLog2, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kMax, 2, IntrinsicDataType::kFloatOrIntScalarOrVector, 0,
     true},
    {ast::Intrinsic::kMin, 2, IntrinsicDataType::kFloatOrIntScalarOrVector, 0,
     true},
    {ast::Intrinsic::kMix, 3, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kModf, 2, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kNormalize, 1, IntrinsicDataType::kFloatVector, 0, true},
    {ast::Intrinsic::kPow, 2, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kReflect, 2, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kReverseBits, 1, IntrinsicDataType::kIntScalarOrVector, 0,
     true},
    {ast::Intrinsic::kRound, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kSign, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kSin, 1, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kSinh, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kSmoothStep, 3, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kSqrt, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kStep, 2, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kTan, 1, IntrinsicDataType::kFloatScalarOrVector, 0, true},
    {ast::Intrinsic::kTanh, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
    {ast::Intrinsic::kTrunc, 1, IntrinsicDataType::kFloatScalarOrVector, 0,
     true},
};

constexpr const uint32_t kIntrinsicDataCount =
    sizeof(kIntrinsicData) / sizeof(IntrinsicData);

bool IsValidType(type::Type* type,
                 const Source& source,
                 const std::string& name,
                 const IntrinsicDataType& data_type,
                 uint32_t vector_size,
                 ValidatorImpl* impl) {
  type = type->UnwrapPtrIfNeeded();
  switch (data_type) {
    case IntrinsicDataType::kFloatOrIntScalarOrVector:
      if (!type->is_float_scalar_or_vector() &&
          !type->is_integer_scalar_or_vector()) {
        impl->add_error(source,
                        "incorrect type for " + name +
                            ". Requires int or float, scalar or vector value");
        return false;
      }
      break;
    case IntrinsicDataType::kFloatScalarOrVector:
      if (!type->is_float_scalar_or_vector()) {
        impl->add_error(source, "incorrect type for " + name +
                                    ". Requires float scalar or vector value");
        return false;
      }
      break;
    case IntrinsicDataType::kIntScalarOrVector:
      if (!type->is_integer_scalar_or_vector()) {
        impl->add_error(source, "incorrect type for " + name +
                                    ". Requires int scalar or vector value");
        return false;
      }
      break;
    case IntrinsicDataType::kFloatVector:
      if (!type->is_float_vector()) {
        impl->add_error(source, "incorrect type for " + name +
                                    ". Requires float vector value");
        return false;
      }
      if (vector_size > 0 && vector_size != type->As<type::Vector>()->size()) {
        impl->add_error(source, "incorrect vector size for " + name +
                                    ". Requires " +
                                    std::to_string(vector_size) + " elements");
        return false;
      }
      break;
    case IntrinsicDataType::kFloatScalar:
      if (!type->Is<type::F32>()) {
        impl->add_error(source, "incorrect type for " + name +
                                    ". Requires float scalar value");
        return false;
      }
      break;
    case IntrinsicDataType::kMatrix:
      if (!type->Is<type::Matrix>()) {
        impl->add_error(
            source, "incorrect type for " + name + ". Requires matrix value");
        return false;
      }
      break;
    default:
      break;
  }

  return true;
}

}  // namespace

ValidatorImpl::ValidatorImpl(const Program* program) : program_(program) {}

ValidatorImpl::~ValidatorImpl() = default;

void ValidatorImpl::add_error(const Source& src,
                              const char* code,
                              const std::string& msg) {
  diag::Diagnostic diag;
  diag.severity = diag::Severity::Error;
  diag.source = src;
  diag.message = msg;
  diag.code = code;
  diags_.add(std::move(diag));
}

void ValidatorImpl::add_error(const Source& src, const std::string& msg) {
  diags_.add_error(msg, src);
}

bool ValidatorImpl::Validate() {
  function_stack_.push_scope();
  if (!ValidateGlobalVariables(program_->AST().GlobalVariables())) {
    return false;
  }
  if (!ValidateConstructedTypes(program_->AST().ConstructedTypes())) {
    return false;
  }
  if (!ValidateFunctions(program_->AST().Functions())) {
    return false;
  }
  if (!ValidateEntryPoint(program_->AST().Functions())) {
    return false;
  }
  function_stack_.pop_scope();

  return true;
}

bool ValidatorImpl::ValidateConstructedTypes(
    const std::vector<type::Type*>& constructed_types) {
  for (auto* const ct : constructed_types) {
    if (auto* st = ct->As<type::Struct>()) {
      for (auto* member : st->impl()->members()) {
        if (auto* r = member->type()->UnwrapAll()->As<type::Array>()) {
          if (r->IsRuntimeArray()) {
            if (member != st->impl()->members().back()) {
              add_error(member->source(), "v-0015",
                        "runtime arrays may only appear as the last member of "
                        "a struct");
              return false;
            }
            if (!st->IsBlockDecorated()) {
              add_error(member->source(), "v-0031",
                        "a struct containing a runtime-sized array "
                        "must be in the 'storage' storage class: '" +
                            program_->Symbols().NameFor(st->symbol()) + "'");
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}

bool ValidatorImpl::ValidateGlobalVariables(
    const ast::VariableList& global_vars) {
  for (auto* var : global_vars) {
    if (variable_stack_.has(var->symbol())) {
      add_error(var->source(), "v-0011",
                "redeclared global identifier '" +
                    program_->Symbols().NameFor(var->symbol()) + "'");
      return false;
    }
    if (!var->is_const() && var->storage_class() == ast::StorageClass::kNone) {
      add_error(var->source(), "v-0022",
                "global variables must have a storage class");
      return false;
    }
    if (var->is_const() &&
        !(var->storage_class() == ast::StorageClass::kNone)) {
      add_error(var->source(), "v-global01",
                "global constants shouldn't have a storage class");
      return false;
    }
    variable_stack_.set_global(var->symbol(), var);
  }
  return true;
}

bool ValidatorImpl::ValidateFunctions(const ast::FunctionList& funcs) {
  for (auto* func : funcs) {
    if (function_stack_.has(func->symbol())) {
      add_error(func->source(), "v-0016",
                "function names must be unique '" +
                    program_->Symbols().NameFor(func->symbol()) + "'");
      return false;
    }

    function_stack_.set(func->symbol(), func);
    current_function_ = func;
    if (!ValidateFunction(func)) {
      return false;
    }
    current_function_ = nullptr;
  }

  return true;
}

bool ValidatorImpl::ValidateEntryPoint(const ast::FunctionList& funcs) {
  auto shader_is_present = false;
  for (auto* func : funcs) {
    if (func->IsEntryPoint()) {
      shader_is_present = true;
      if (!func->params().empty()) {
        add_error(func->source(), "v-0023",
                  "Entry point function must accept no parameters: '" +
                      program_->Symbols().NameFor(func->symbol()) + "'");
        return false;
      }

      if (!func->return_type()->Is<type::Void>()) {
        add_error(func->source(), "v-0024",
                  "Entry point function must return void: '" +
                      program_->Symbols().NameFor(func->symbol()) + "'");
        return false;
      }
      auto stage_deco_count = 0;
      for (auto* deco : func->decorations()) {
        if (deco->Is<ast::StageDecoration>()) {
          stage_deco_count++;
        }
      }
      if (stage_deco_count > 1) {
        add_error(func->source(), "v-0020",
                  "only one stage decoration permitted per entry point");
        return false;
      }
    }
  }
  if (!shader_is_present) {
    add_error(Source{}, "v-0003",
              "At least one of vertex, fragment or compute shader must "
              "be present");
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateFunction(const ast::Function* func) {
  variable_stack_.push_scope();

  for (auto* param : func->params()) {
    variable_stack_.set(param->symbol(), param);
    if (!ValidateParameter(param)) {
      return false;
    }
  }
  if (!ValidateStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  if (!current_function_->return_type()->Is<type::Void>()) {
    if (!func->get_last_statement() ||
        !func->get_last_statement()->Is<ast::ReturnStatement>()) {
      add_error(func->source(), "v-0002",
                "non-void function must end with a return statement");
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateParameter(const ast::Variable* param) {
  if (auto* r = param->type()->UnwrapAll()->As<type::Array>()) {
    if (r->IsRuntimeArray()) {
      add_error(
          param->source(), "v-0015",
          "runtime arrays may only appear as the last member of a struct");
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateReturnStatement(const ast::ReturnStatement* ret) {
  // TODO(sarahM0): update this when this issue resolves:
  // https://github.com/gpuweb/gpuweb/issues/996
  type::Type* func_type = current_function_->return_type();

  type::Void void_type;
  auto* ret_type = ret->has_value()
                       ? program_->Sem().Get(ret->value())->Type()->UnwrapAll()
                       : &void_type;

  if (func_type->type_name() != ret_type->type_name()) {
    add_error(ret->source(), "v-000y",
              "return statement type must match its function return "
              "type, returned '" +
                  ret_type->type_name() + "', expected '" +
                  func_type->type_name() + "'");
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateStatements(const ast::BlockStatement* block) {
  if (!block) {
    return false;
  }
  for (auto* stmt : *block) {
    if (!ValidateStatement(stmt)) {
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateDeclStatement(
    const ast::VariableDeclStatement* decl) {
  auto symbol = decl->variable()->symbol();
  bool is_global = false;
  if (variable_stack_.get(symbol, nullptr, &is_global)) {
    const char* error_code = "v-0014";
    if (is_global) {
      error_code = "v-0013";
    }
    add_error(
        decl->source(), error_code,
        "redeclared identifier '" + program_->Symbols().NameFor(symbol) + "'");
    return false;
  }
  // TODO(dneto): Check type compatibility of the initializer.
  //  - if it's non-constant, then is storable or can be dereferenced to be
  //    storable.
  //  - types match or the RHS can be dereferenced to equal the LHS type.
  variable_stack_.set(symbol, decl->variable());
  if (auto* arr = decl->variable()->type()->UnwrapAll()->As<type::Array>()) {
    if (arr->IsRuntimeArray()) {
      add_error(
          decl->source(), "v-0015",
          "runtime arrays may only appear as the last member of a struct");
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateStatement(const ast::Statement* stmt) {
  if (!stmt) {
    return false;
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    bool constructor_valid =
        v->variable()->has_constructor()
            ? ValidateExpression(v->variable()->constructor())
            : true;

    return constructor_valid && ValidateDeclStatement(v);
  }
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return ValidateAssign(a);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return ValidateReturnStatement(r);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    return ValidateCallExpr(c->expr());
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return ValidateSwitch(s);
  }
  if (auto* c = stmt->As<ast::CaseStatement>()) {
    return ValidateCase(c);
  }
  return true;
}

bool ValidatorImpl::ValidateSwitch(const ast::SwitchStatement* s) {
  if (!ValidateExpression(s->condition())) {
    return false;
  }

  auto* cond_type = program_->Sem().Get(s->condition())->Type()->UnwrapAll();
  if (!cond_type->is_integer_scalar()) {
    add_error(s->condition()->source(), "v-0025",
              "switch statement selector expression must be of a "
              "scalar integer type");
    return false;
  }

  int default_counter = 0;
  std::unordered_set<int32_t> selector_set;
  for (auto* case_stmt : s->body()) {
    if (!ValidateStatement(case_stmt)) {
      return false;
    }

    if (case_stmt->IsDefault()) {
      default_counter++;
    }

    for (auto* selector : case_stmt->selectors()) {
      if (cond_type != selector->type()) {
        add_error(case_stmt->source(), "v-0026",
                  "the case selector values must have the same "
                  "type as the selector expression.");
        return false;
      }

      auto v =
          static_cast<int32_t>(selector->type()->Is<type::U32>()
                                   ? selector->As<ast::UintLiteral>()->value()
                                   : selector->As<ast::SintLiteral>()->value());
      if (selector_set.count(v)) {
        add_error(case_stmt->source(), "v-0027",
                  "a literal value must not appear more than once in "
                  "the case selectors for a switch statement: '" +
                      program_->str(selector) + "'");
        return false;
      }
      selector_set.emplace(v);
    }
  }

  if (default_counter != 1) {
    add_error(s->source(), "v-0008",
              "switch statement must have exactly one default clause");
    return false;
  }

  auto* last_clause = s->body().back();
  auto* last_stmt_of_last_clause =
      last_clause->As<ast::CaseStatement>()->body()->last();
  if (last_stmt_of_last_clause &&
      last_stmt_of_last_clause->Is<ast::FallthroughStatement>()) {
    add_error(last_stmt_of_last_clause->source(), "v-0028",
              "a fallthrough statement must not appear as "
              "the last statement in last clause of a switch");
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateCase(const ast::CaseStatement* c) {
  if (!ValidateStatement(c->body())) {
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateCallExpr(const ast::CallExpression* expr) {
  if (!expr) {
    // TODO(sarahM0): Here and other Validate.*: figure out whether return
    // false or true
    return false;
  }

  if (auto* ident = expr->func()->As<ast::IdentifierExpression>()) {
    auto symbol = ident->symbol();
    if (ident->IsIntrinsic()) {
      const IntrinsicData* data = nullptr;
      for (uint32_t i = 0; i < kIntrinsicDataCount; ++i) {
        if (ident->intrinsic() == kIntrinsicData[i].intrinsic) {
          data = &kIntrinsicData[i];
          break;
        }
      }

      if (data != nullptr) {
        const auto builtin = program_->Symbols().NameFor(symbol);
        if (expr->params().size() != data->param_count) {
          add_error(expr->source(),
                    "incorrect number of parameters for " + builtin +
                        " expected " + std::to_string(data->param_count) +
                        " got " + std::to_string(expr->params().size()));
          return false;
        }

        if (data->all_types_match) {
          // Check that the type is an acceptable one.
          if (!IsValidType(program_->TypeOf(expr->func()), expr->source(),
                           builtin, data->data_type, data->vector_size, this)) {
            return false;
          }

          // Check that all params match the result type.
          for (uint32_t i = 0; i < data->param_count; ++i) {
            if (program_->TypeOf(expr->func())->UnwrapPtrIfNeeded() !=
                program_->TypeOf(expr->params()[i])->UnwrapPtrIfNeeded()) {
              add_error(expr->params()[i]->source(),
                        "expected parameter " + std::to_string(i) +
                            "'s unwrapped type to match result type for " +
                            builtin);
              return false;
            }
          }
        } else {
          if (data->data_type != IntrinsicDataType::kMixed) {
            auto* p0 = expr->params()[0];
            if (!IsValidType(program_->TypeOf(p0), p0->source(), builtin,
                             data->data_type, data->vector_size, this)) {
              return false;
            }

            // Check that parameters are valid types.
            for (uint32_t i = 1; i < expr->params().size(); ++i) {
              if (program_->TypeOf(p0)->UnwrapPtrIfNeeded() !=
                  program_->TypeOf(expr->params()[i])->UnwrapPtrIfNeeded()) {
                add_error(
                    expr->source(),
                    "parameter " + std::to_string(i) +
                        "'s unwrapped type must match parameter 0's type");
                return false;
              }
            }
          } else {
            // Special cases.
            if (data->intrinsic == ast::Intrinsic::kFrexp) {
              auto* p0 = expr->params()[0];
              auto* p1 = expr->params()[1];
              auto* t0 = program_->TypeOf(p0)->UnwrapPtrIfNeeded();
              auto* t1 = program_->TypeOf(p1)->UnwrapPtrIfNeeded();
              if (!IsValidType(t0, p0->source(), builtin,
                               IntrinsicDataType::kFloatScalarOrVector, 0,
                               this)) {
                return false;
              }
              if (!IsValidType(t1, p1->source(), builtin,
                               IntrinsicDataType::kIntScalarOrVector, 0,
                               this)) {
                return false;
              }

              if (t0->is_scalar()) {
                if (!t1->is_scalar()) {
                  add_error(
                      expr->source(),
                      "incorrect types for " + builtin +
                          ". Parameters must be matched scalars or vectors");
                  return false;
                }
              } else {
                if (t1->is_integer_scalar()) {
                  add_error(
                      expr->source(),
                      "incorrect types for " + builtin +
                          ". Parameters must be matched scalars or vectors");
                  return false;
                }
                const auto* v0 = t0->As<type::Vector>();
                const auto* v1 = t1->As<type::Vector>();
                if (v0->size() != v1->size()) {
                  add_error(expr->source(),
                            "incorrect types for " + builtin +
                                ". Parameter vector sizes must match");
                  return false;
                }
              }
            }
          }

          // Result types don't match parameter types.
          if (data->intrinsic == ast::Intrinsic::kLength ||
              data->intrinsic == ast::Intrinsic::kDistance ||
              data->intrinsic == ast::Intrinsic::kDeterminant) {
            if (!IsValidType(program_->TypeOf(expr->func()), expr->source(),
                             builtin, IntrinsicDataType::kFloatScalar, 0,
                             this)) {
              return false;
            }
          }

          // Must be a square matrix.
          if (data->intrinsic == ast::Intrinsic::kDeterminant) {
            const auto* matrix =
                program_->TypeOf(expr->params()[0])->As<type::Matrix>();
            if (matrix->rows() != matrix->columns()) {
              add_error(expr->params()[0]->source(),
                        "incorrect type for " + builtin +
                            ". Requires a square matrix");
              return false;
            }
          }
        }

        // Last parameter must be a pointer.
        if (data->intrinsic == ast::Intrinsic::kFrexp ||
            data->intrinsic == ast::Intrinsic::kModf) {
          auto* last_param = expr->params()[data->param_count - 1];
          if (!program_->TypeOf(last_param)->Is<type::Pointer>()) {
            add_error(last_param->source(), "incorrect type for " + builtin +
                                                ". Requires pointer value");
            return false;
          }
        }
      }
    } else {
      if (!function_stack_.has(symbol)) {
        add_error(expr->source(), "v-0005",
                  "function must be declared before use: '" +
                      program_->Symbols().NameFor(symbol) + "'");
        return false;
      }
      if (symbol == current_function_->symbol()) {
        add_error(expr->source(), "v-0004",
                  "recursion is not allowed: '" +
                      program_->Symbols().NameFor(symbol) + "'");
        return false;
      }
    }
  } else {
    add_error(expr->source(), "Invalid function call expression");
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateBadAssignmentToIdentifier(
    const ast::AssignmentStatement* assign) {
  auto* ident = assign->lhs()->As<ast::IdentifierExpression>();
  if (!ident) {
    // It wasn't an identifier in the first place.
    return true;
  }
  ast::Variable* var;
  if (variable_stack_.get(ident->symbol(), &var)) {
    // Give a nicer message if the LHS of the assignment is a const identifier.
    // It's likely to be a common programmer error.
    if (var->is_const()) {
      add_error(assign->source(), "v-0021",
                "cannot re-assign a constant: '" +
                    program_->Symbols().NameFor(ident->symbol()) + "'");
      return false;
    }
  } else {
    // The identifier is not defined. This should already have been caught
    // when validating the subexpression.
    add_error(ident->source(), "v-0006",
              "'" + program_->Symbols().NameFor(ident->symbol()) +
                  "' is not declared");
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateAssign(const ast::AssignmentStatement* assign) {
  if (!assign) {
    return false;
  }
  auto* lhs = assign->lhs();
  auto* rhs = assign->rhs();
  if (!ValidateExpression(lhs)) {
    return false;
  }
  if (!ValidateExpression(rhs)) {
    return false;
  }
  // Pointers are not storable in WGSL, but the right-hand side must be
  // storable. The raw right-hand side might be a pointer value which must be
  // loaded (dereferenced) to provide the value to be stored.
  auto* rhs_result_type = program_->Sem().Get(rhs)->Type()->UnwrapAll();
  if (!IsStorable(rhs_result_type)) {
    add_error(assign->source(), "v-000x",
              "invalid assignment: right-hand-side is not storable: " +
                  program_->Sem().Get(rhs)->Type()->type_name());
    return false;
  }
  auto* lhs_result_type = program_->Sem().Get(lhs)->Type()->UnwrapIfNeeded();
  if (auto* lhs_reference_type = As<type::Pointer>(lhs_result_type)) {
    auto* lhs_store_type = lhs_reference_type->type()->UnwrapIfNeeded();
    if (lhs_store_type != rhs_result_type) {
      add_error(assign->source(), "v-000x",
                "invalid assignment: can't assign value of type '" +
                    rhs_result_type->type_name() + "' to '" +
                    lhs_store_type->type_name() + "'");
      return false;
    }
  } else {
    if (!ValidateBadAssignmentToIdentifier(assign)) {
      return false;
    }
    // Issue a generic error.
    add_error(
        assign->source(), "v-000x",
        "invalid assignment: left-hand-side does not reference storage: " +
            program_->Sem().Get(lhs)->Type()->type_name());
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateExpression(const ast::Expression* expr) {
  if (!expr) {
    return false;
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return ValidateIdentifier(i);
  }

  if (auto* c = expr->As<ast::CallExpression>()) {
    return ValidateCallExpr(c);
  }
  return true;
}

bool ValidatorImpl::ValidateIdentifier(const ast::IdentifierExpression* ident) {
  ast::Variable* var;
  if (!variable_stack_.get(ident->symbol(), &var)) {
    add_error(ident->source(), "v-0006",
              "'" + program_->Symbols().NameFor(ident->symbol()) +
                  "' is not declared");
    return false;
  }
  return true;
}

bool ValidatorImpl::IsStorable(type::Type* type) {
  if (type == nullptr) {
    return false;
  }
  if (type->is_scalar() || type->Is<type::Vector>() ||
      type->Is<type::Matrix>()) {
    return true;
  }
  if (type::Array* array_type = type->As<type::Array>()) {
    return IsStorable(array_type->type());
  }
  if (type::Struct* struct_type = type->As<type::Struct>()) {
    for (const auto* member : struct_type->impl()->members()) {
      if (!IsStorable(member->type())) {
        return false;
      }
    }
    return true;
  }
  if (type::Alias* alias_type = type->As<type::Alias>()) {
    return IsStorable(alias_type->type());
  }
  return false;
}

}  // namespace tint
