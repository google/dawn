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

#include "src/writer/msl/generator_impl.h"

#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/continue_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic.h"
#include "src/ast/location_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/sint_literal.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

const char kInStructNameSuffix[] = "in";
const char kOutStructNameSuffix[] = "out";
const char kTintStructInVarPrefix[] = "tint_in";
const char kTintStructOutVarPrefix[] = "tint_out";

bool last_is_break_or_fallthrough(const ast::StatementList& stmts) {
  if (stmts.empty()) {
    return false;
  }

  return stmts.back()->IsBreak() || stmts.back()->IsFallthrough();
}

}  // namespace

GeneratorImpl::GeneratorImpl() = default;

GeneratorImpl::~GeneratorImpl() = default;

void GeneratorImpl::set_module_for_testing(ast::Module* mod) {
  module_ = mod;
}

std::string GeneratorImpl::generate_name(const std::string& prefix) {
  std::string name = prefix;
  uint32_t i = 0;
  while (namer_.IsMapped(name)) {
    name = prefix + "_" + std::to_string(i);
    ++i;
  }
  namer_.RegisterRemappedName(name);
  return name;
}

bool GeneratorImpl::Generate(const ast::Module& module) {
  module_ = &module;

  out_ << "#include <metal_stdlib>" << std::endl << std::endl;

  for (const auto& global : module.global_variables()) {
    global_variables_.set(global->name(), global.get());
  }

  for (auto* const alias : module.alias_types()) {
    if (!EmitAliasType(alias)) {
      return false;
    }
  }
  if (!module.alias_types().empty()) {
    out_ << std::endl;
  }

  for (const auto& ep : module.entry_points()) {
    if (!EmitEntryPointData(ep.get())) {
      return false;
    }
  }

  for (const auto& func : module.functions()) {
    if (!EmitFunction(func.get())) {
      return false;
    }
  }

  for (const auto& ep : module.entry_points()) {
    if (!EmitEntryPointFunction(ep.get())) {
      return false;
    }
    out_ << std::endl;
  }

  module_ = nullptr;
  return true;
}

bool GeneratorImpl::EmitAliasType(const ast::type::AliasType* alias) {
  make_indent();
  out_ << "typedef ";
  if (!EmitType(alias->type(), "")) {
    return false;
  }
  out_ << " " << namer_.NameFor(alias->name()) << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitArrayAccessor(ast::ArrayAccessorExpression* expr) {
  if (!EmitExpression(expr->array())) {
    return false;
  }
  out_ << "[";

  if (!EmitExpression(expr->idx_expr())) {
    return false;
  }
  out_ << "]";

  return true;
}

bool GeneratorImpl::EmitAs(ast::AsExpression* expr) {
  out_ << "as_type<";
  if (!EmitType(expr->type(), "")) {
    return false;
  }

  out_ << ">(";
  if (!EmitExpression(expr->expr())) {
    return false;
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(ast::AssignmentStatement* stmt) {
  make_indent();

  if (!EmitExpression(stmt->lhs())) {
    return false;
  }

  out_ << " = ";

  if (!EmitExpression(stmt->rhs())) {
    return false;
  }

  out_ << ";" << std::endl;

  return true;
}

bool GeneratorImpl::EmitBinary(ast::BinaryExpression* expr) {
  out_ << "(";

  if (!EmitExpression(expr->lhs())) {
    return false;
  }
  out_ << " ";

  switch (expr->op()) {
    case ast::BinaryOp::kAnd:
      out_ << "&";
      break;
    case ast::BinaryOp::kOr:
      out_ << "|";
      break;
    case ast::BinaryOp::kXor:
      out_ << "^";
      break;
    case ast::BinaryOp::kLogicalAnd:
      out_ << "&&";
      break;
    case ast::BinaryOp::kLogicalOr:
      out_ << "||";
      break;
    case ast::BinaryOp::kEqual:
      out_ << "==";
      break;
    case ast::BinaryOp::kNotEqual:
      out_ << "!=";
      break;
    case ast::BinaryOp::kLessThan:
      out_ << "<";
      break;
    case ast::BinaryOp::kGreaterThan:
      out_ << ">";
      break;
    case ast::BinaryOp::kLessThanEqual:
      out_ << "<=";
      break;
    case ast::BinaryOp::kGreaterThanEqual:
      out_ << ">=";
      break;
    case ast::BinaryOp::kShiftLeft:
      out_ << "<<";
      break;
    case ast::BinaryOp::kShiftRight:
      // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
      // implementation-defined behaviour for negative LHS.  We may have to
      // generate extra code to implement WGSL-specified behaviour for negative
      // LHS.
      out_ << R"(>>)";
      break;

    case ast::BinaryOp::kAdd:
      out_ << "+";
      break;
    case ast::BinaryOp::kSubtract:
      out_ << "-";
      break;
    case ast::BinaryOp::kMultiply:
      out_ << "*";
      break;
    case ast::BinaryOp::kDivide:
      out_ << "/";
      break;
    case ast::BinaryOp::kModulo:
      out_ << "%";
      break;
    case ast::BinaryOp::kNone:
      error_ = "missing binary operation type";
      return false;
  }
  out_ << " ";

  if (!EmitExpression(expr->rhs())) {
    return false;
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitBreak(ast::BreakStatement*) {
  make_indent();
  out_ << "break;" << std::endl;
  return true;
}

std::string GeneratorImpl::current_ep_var_name(VarType type) {
  std::string name = "";
  switch (type) {
    case VarType::kIn: {
      auto in_it = ep_name_to_in_data_.find(current_ep_name_);
      if (in_it != ep_name_to_in_data_.end()) {
        name = in_it->second.var_name;
      }
      break;
    }
    case VarType::kOut: {
      auto out_it = ep_name_to_out_data_.find(current_ep_name_);
      if (out_it != ep_name_to_out_data_.end()) {
        name = out_it->second.var_name;
      }
      break;
    }
  }
  return name;
}

bool GeneratorImpl::EmitCall(ast::CallExpression* expr) {
  if (!expr->func()->IsIdentifier()) {
    error_ = "invalid function name";
    return 0;
  }

  auto* ident = expr->func()->AsIdentifier();

  if (!ident->has_path() && ast::intrinsic::IsIntrinsic(ident->name())) {
    // TODO(dsinclair): Generate intrinsic
    error_ = "intrinsics not generated yet";
    return false;
  }

  if (!ident->has_path()) {
    auto name = ident->name();
    auto it = ep_func_name_remapped_.find(current_ep_name_ + "_" + name);
    if (it != ep_func_name_remapped_.end()) {
      name = it->second;
    }
    out_ << name << "(";

    bool first = true;
    auto var_name = current_ep_var_name(VarType::kIn);
    if (!var_name.empty()) {
      out_ << var_name;
      first = false;
    }

    var_name = current_ep_var_name(VarType::kOut);
    if (!var_name.empty()) {
      if (!first) {
        out_ << ", ";
      }
      first = false;
      out_ << var_name;
    }

    auto* func = module_->FindFunctionByName(ident->name());
    if (func == nullptr) {
      error_ = "Unable to find function: " + name;
      return false;
    }
    for (const auto& data : func->referenced_builtin_variables()) {
      auto* var = data.first;
      if (var->storage_class() != ast::StorageClass::kInput) {
        continue;
      }
      if (!first) {
        out_ << ", ";
      }
      first = false;
      out_ << var->name();
    }

    const auto& params = expr->params();
    for (const auto& param : params) {
      if (!first) {
        out_ << ", ";
      }
      first = false;

      if (!EmitExpression(param.get())) {
        return false;
      }
    }

    out_ << ")";
  } else {
    // TODO(dsinclair): Handle imported function
    error_ = "imported calls not handled yet";
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitCase(ast::CaseStatement* stmt) {
  make_indent();

  if (stmt->IsDefault()) {
    out_ << "default:";
  } else {
    bool first = true;
    for (const auto& selector : stmt->selectors()) {
      if (!first) {
        out_ << std::endl;
        make_indent();
      }
      first = false;

      out_ << "case ";
      if (!EmitLiteral(selector.get())) {
        return false;
      }
      out_ << ":";
    }
  }

  out_ << " {" << std::endl;

  increment_indent();

  for (const auto& s : stmt->body()) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }

  if (!last_is_break_or_fallthrough(stmt->body())) {
    make_indent();
    out_ << "break;" << std::endl;
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitCast(ast::CastExpression* expr) {
  if (!EmitType(expr->type(), "")) {
    return false;
  }

  out_ << "(";
  if (!EmitExpression(expr->expr())) {
    return false;
  }
  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitConstructor(ast::ConstructorExpression* expr) {
  if (expr->IsScalarConstructor()) {
    return EmitScalarConstructor(expr->AsScalarConstructor());
  }
  return EmitTypeConstructor(expr->AsTypeConstructor());
}

bool GeneratorImpl::EmitContinue(ast::ContinueStatement*) {
  make_indent();
  out_ << "continue;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitTypeConstructor(ast::TypeConstructorExpression* expr) {
  if (!EmitType(expr->type(), "")) {
    return false;
  }

  out_ << "(";

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (expr->values().empty()) {
    if (!EmitZeroValue(expr->type())) {
      return false;
    }
  } else {
    bool first = true;
    for (const auto& e : expr->values()) {
      if (!first) {
        out_ << ", ";
      }
      first = false;

      if (!EmitExpression(e.get())) {
        return false;
      }
    }
  }

  out_ << ")";
  return true;
}

bool GeneratorImpl::EmitZeroValue(ast::type::Type* type) {
  if (type->IsBool()) {
    out_ << "false";
  } else if (type->IsF32()) {
    out_ << "0.0f";
  } else if (type->IsI32()) {
    out_ << "0";
  } else if (type->IsU32()) {
    out_ << "0u";
  } else if (type->IsVector()) {
    return EmitZeroValue(type->AsVector()->type());
  } else {
    error_ = "Invalid type for zero emission: " + type->type_name();
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitScalarConstructor(
    ast::ScalarConstructorExpression* expr) {
  return EmitLiteral(expr->literal());
}

bool GeneratorImpl::EmitLiteral(ast::Literal* lit) {
  if (lit->IsBool()) {
    out_ << (lit->AsBool()->IsTrue() ? "true" : "false");
  } else if (lit->IsFloat()) {
    auto flags = out_.flags();
    auto precision = out_.precision();

    out_.flags(flags | std::ios_base::showpoint);
    out_.precision(std::numeric_limits<float>::max_digits10);

    out_ << lit->AsFloat()->value() << "f";

    out_.precision(precision);
    out_.flags(flags);
  } else if (lit->IsSint()) {
    out_ << lit->AsSint()->value();
  } else if (lit->IsUint()) {
    out_ << lit->AsUint()->value() << "u";
  } else {
    error_ = "unknown literal type";
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitEntryPointData(ast::EntryPoint* ep) {
  auto* func = module_->FindFunctionByName(ep->function_name());
  if (func == nullptr) {
    error_ = "Unable to find entry point function: " + ep->function_name();
    return false;
  }

  std::vector<std::pair<ast::Variable*, uint32_t>> in_locations;
  std::vector<std::pair<ast::Variable*, ast::VariableDecoration*>>
      out_variables;
  for (auto data : func->referenced_location_variables()) {
    auto var = data.first;
    auto deco = data.second;

    if (var->storage_class() == ast::StorageClass::kInput) {
      in_locations.push_back({var, deco->value()});
    } else if (var->storage_class() == ast::StorageClass::kOutput) {
      out_variables.push_back({var, deco});
    }
  }

  for (auto data : func->referenced_builtin_variables()) {
    auto var = data.first;
    auto deco = data.second;

    if (var->storage_class() == ast::StorageClass::kOutput) {
      out_variables.push_back({var, deco});
    }
  }

  auto ep_name = ep->name();
  if (ep_name.empty()) {
    ep_name = ep->function_name();
  }

  // TODO(dsinclair): There is a potential bug here. Entry points can have the
  // same name in WGSL if they have different pipeline stages. This does not
  // take that into account and will emit duplicate struct names. I'm ignoring
  // this until https://github.com/gpuweb/gpuweb/issues/662 is resolved as it
  // may remove this issue and entry point names will need to be unique.
  if (!in_locations.empty()) {
    auto in_struct_name = generate_name(ep_name + "_" + kInStructNameSuffix);
    auto in_var_name = generate_name(kTintStructInVarPrefix);
    ep_name_to_in_data_[ep_name] = {in_struct_name, in_var_name};

    make_indent();
    out_ << "struct " << in_struct_name << " {" << std::endl;

    increment_indent();

    for (auto& data : in_locations) {
      auto* var = data.first;
      uint32_t loc = data.second;

      make_indent();
      if (!EmitType(var->type(), var->name())) {
        return false;
      }

      out_ << " " << var->name() << " [[";
      if (ep->stage() == ast::PipelineStage::kVertex) {
        out_ << "attribute(" << loc << ")";
      } else if (ep->stage() == ast::PipelineStage::kFragment) {
        out_ << "user(locn" << loc << ")";
      } else {
        error_ = "invalid location variable for pipeline stage";
        return false;
      }
      out_ << "]];" << std::endl;
    }
    decrement_indent();
    make_indent();

    out_ << "};" << std::endl << std::endl;
  }

  if (!out_variables.empty()) {
    auto out_struct_name = generate_name(ep_name + "_" + kOutStructNameSuffix);
    auto out_var_name = generate_name(kTintStructOutVarPrefix);
    ep_name_to_out_data_[ep_name] = {out_struct_name, out_var_name};

    make_indent();
    out_ << "struct " << out_struct_name << " {" << std::endl;

    increment_indent();
    for (auto& data : out_variables) {
      auto* var = data.first;
      auto* deco = data.second;

      make_indent();
      if (!EmitType(var->type(), var->name())) {
        return false;
      }

      out_ << " " << var->name() << " [[";

      if (deco->IsLocation()) {
        auto loc = deco->AsLocation()->value();
        if (ep->stage() == ast::PipelineStage::kVertex) {
          out_ << "user(locn" << loc << ")";
        } else if (ep->stage() == ast::PipelineStage::kFragment) {
          out_ << "color(" << loc << ")";
        } else {
          error_ = "invalid location variable for pipeline stage";
          return false;
        }
      } else if (deco->IsBuiltin()) {
        auto attr = builtin_to_attribute(deco->AsBuiltin()->value());
        if (attr.empty()) {
          error_ = "unsupported builtin";
          return false;
        }
        out_ << attr;
      } else {
        error_ = "unsupported variable decoration for entry point output";
        return false;
      }
      out_ << "]];" << std::endl;
    }
    decrement_indent();
    make_indent();
    out_ << "};" << std::endl << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (expr->IsArrayAccessor()) {
    return EmitArrayAccessor(expr->AsArrayAccessor());
  }
  if (expr->IsAs()) {
    return EmitAs(expr->AsAs());
  }
  if (expr->IsBinary()) {
    return EmitBinary(expr->AsBinary());
  }
  if (expr->IsCall()) {
    return EmitCall(expr->AsCall());
  }
  if (expr->IsCast()) {
    return EmitCast(expr->AsCast());
  }
  if (expr->IsConstructor()) {
    return EmitConstructor(expr->AsConstructor());
  }
  if (expr->IsIdentifier()) {
    return EmitIdentifier(expr->AsIdentifier());
  }
  if (expr->IsMemberAccessor()) {
    return EmitMemberAccessor(expr->AsMemberAccessor());
  }
  if (expr->IsUnaryOp()) {
    return EmitUnaryOp(expr->AsUnaryOp());
  }

  error_ = "unknown expression type: " + expr->str();
  return false;
}

void GeneratorImpl::EmitStage(ast::PipelineStage stage) {
  switch (stage) {
    case ast::PipelineStage::kFragment:
      out_ << "fragment";
      break;
    case ast::PipelineStage::kVertex:
      out_ << "vertex";
      break;
    case ast::PipelineStage::kCompute:
      out_ << "kernel";
      break;
    case ast::PipelineStage::kNone:
      break;
  }
  return;
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  make_indent();

  // Entry points will be emitted later, skip for now.
  if (module_->IsFunctionEntryPoint(func->name())) {
    return true;
  }

  // TODO(dsinclair): This could be smarter. If the input/outputs for multiple
  // entry points are the same we could generate a single struct and then have
  // this determine it's the same struct and just emit once.
  bool emit_duplicate_functions =
      func->ancestor_entry_points().size() > 0 &&
      func->referenced_module_variables().size() > 0;

  if (emit_duplicate_functions) {
    for (const auto& ep_name : func->ancestor_entry_points()) {
      if (!EmitFunctionInternal(func, emit_duplicate_functions, ep_name)) {
        return false;
      }
      out_ << std::endl;
    }
  } else {
    // Emit as non-duplicated
    if (!EmitFunctionInternal(func, false, "")) {
      return false;
    }
    out_ << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitFunctionInternal(ast::Function* func,
                                         bool emit_duplicate_functions,
                                         const std::string& ep_name) {
  auto name = func->name();

  if (!EmitType(func->return_type(), "")) {
    return false;
  }

  out_ << " ";

  if (emit_duplicate_functions) {
    name = generate_name(name + "_" + ep_name);
    ep_func_name_remapped_[ep_name + "_" + func->name()] = name;
  } else {
    name = namer_.NameFor(name);
  }
  out_ << name << "(";

  bool first = true;

  // If we're emitting duplicate functions that means the function takes
  // the stage_in or stage_out value from the entry point, emit them.
  //
  // We emit both of them if they're there regardless of if they're both used.
  if (emit_duplicate_functions) {
    auto in_it = ep_name_to_in_data_.find(ep_name);
    if (in_it != ep_name_to_in_data_.end()) {
      out_ << "thread " << in_it->second.struct_name << "& "
           << in_it->second.var_name;
      first = false;
    }

    auto out_it = ep_name_to_out_data_.find(ep_name);
    if (out_it != ep_name_to_out_data_.end()) {
      if (!first) {
        out_ << ", ";
      }
      out_ << "thread " << out_it->second.struct_name << "& "
           << out_it->second.var_name;
      first = false;
    }
  }

  for (const auto& data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    if (var->storage_class() != ast::StorageClass::kInput) {
      continue;
    }
    if (!first) {
      out_ << ", ";
    }
    first = false;

    out_ << "thread ";
    if (!EmitType(var->type(), "")) {
      return false;
    }
    out_ << "& " << var->name();
  }

  // TODO(dsinclair): Binding/Set inputs

  for (const auto& v : func->params()) {
    if (!first) {
      out_ << ", ";
    }
    first = false;

    if (!EmitType(v->type(), v->name())) {
      return false;
    }
    // Array name is output as part of the type
    if (!v->type()->IsArray()) {
      out_ << " " << v->name();
    }
  }

  out_ << ")";

  current_ep_name_ = ep_name;

  if (!EmitStatementBlockAndNewline(func->body())) {
    return false;
  }

  current_ep_name_ = "";

  return true;
}

std::string GeneratorImpl::builtin_to_attribute(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "position";
    case ast::Builtin::kVertexIdx:
      return "vertex_id";
    case ast::Builtin::kInstanceIdx:
      return "instance_id";
    case ast::Builtin::kFrontFacing:
      return "front_facing";
    case ast::Builtin::kFragCoord:
      return "position";
    case ast::Builtin::kFragDepth:
      return "depth(any)";
    // TODO(dsinclair): Ignore for now, I believe it will be removed from WGSL
    // https://github.com/gpuweb/gpuweb/issues/920
    case ast::Builtin::kNumWorkgroups:
      return "";
    // TODO(dsinclair): Ignore for now. This has been removed as a builtin
    // in the spec. Need to update Tint to match.
    // https://github.com/gpuweb/gpuweb/pull/824
    case ast::Builtin::kWorkgroupSize:
      return "";
    case ast::Builtin::kLocalInvocationId:
      return "thread_position_in_threadgroup";
    case ast::Builtin::kLocalInvocationIdx:
      return "thread_index_in_threadgroup";
    case ast::Builtin::kGlobalInvocationId:
      return "thread_position_in_grid";
    default:
      break;
  }
  return "";
}

bool GeneratorImpl::EmitEntryPointFunction(ast::EntryPoint* ep) {
  make_indent();

  current_ep_name_ = ep->name();
  if (current_ep_name_.empty()) {
    current_ep_name_ = ep->function_name();
  }

  auto* func = module_->FindFunctionByName(ep->function_name());
  if (func == nullptr) {
    error_ = "unable to find function for entry point: " + ep->function_name();
    return false;
  }

  EmitStage(ep->stage());
  out_ << " ";

  // This is an entry point, the return type is the entry point output structure
  // if one exists, or void otherwise.
  auto out_data = ep_name_to_out_data_.find(current_ep_name_);
  bool has_out_data = out_data != ep_name_to_out_data_.end();
  if (has_out_data) {
    out_ << out_data->second.struct_name;
  } else {
    out_ << "void";
  }
  out_ << " " << namer_.NameFor(current_ep_name_) << "(";

  bool first = true;
  auto in_data = ep_name_to_in_data_.find(current_ep_name_);
  if (in_data != ep_name_to_in_data_.end()) {
    out_ << in_data->second.struct_name << " " << in_data->second.var_name
         << " [[stage_in]]";
    first = false;
  }

  for (auto data : func->referenced_builtin_variables()) {
    auto* var = data.first;
    if (var->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    if (!first) {
      out_ << ", ";
    }
    first = false;

    auto* builtin = data.second;

    if (!EmitType(var->type(), "")) {
      return false;
    }

    auto attr = builtin_to_attribute(builtin->value());
    if (attr.empty()) {
      error_ = "unknown builtin";
      return false;
    }
    out_ << " " << var->name() << " [[" << attr << "]]";
  }

  // TODO(dsinclair): Binding/Set inputs

  out_ << ") {" << std::endl;

  increment_indent();

  if (has_out_data) {
    make_indent();
    out_ << out_data->second.struct_name << " " << out_data->second.var_name
         << " = {};" << std::endl;
  }

  generating_entry_point_ = true;
  for (const auto& s : func->body()) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }
  generating_entry_point_ = false;

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  current_ep_name_ = "";
  return true;
}

bool GeneratorImpl::EmitIdentifier(ast::IdentifierExpression* expr) {
  auto* ident = expr->AsIdentifier();
  if (ident->has_path()) {
    // TODO(dsinclair): Handle identifier with path
    error_ = "Identifier paths not handled yet.";
    return false;
  }

  ast::Variable* var = nullptr;
  if (global_variables_.get(ident->name(), &var)) {
    bool in_or_out_struct_has_location =
        var->IsDecorated() && var->AsDecorated()->HasLocationDecoration() &&
        (var->storage_class() == ast::StorageClass::kInput ||
         var->storage_class() == ast::StorageClass::kOutput);
    bool in_struct_has_builtin =
        var->IsDecorated() && var->AsDecorated()->HasBuiltinDecoration() &&
        var->storage_class() == ast::StorageClass::kOutput;
    if (in_or_out_struct_has_location || in_struct_has_builtin) {
      auto var_type = var->storage_class() == ast::StorageClass::kInput
                          ? VarType::kIn
                          : VarType::kOut;
      auto name = current_ep_var_name(var_type);
      if (name.empty()) {
        error_ = "unable to find entry point data for variable";
        return false;
      }
      out_ << name << ".";
    }
  }
  out_ << namer_.NameFor(ident->name());

  return true;
}

bool GeneratorImpl::EmitLoop(ast::LoopStatement* stmt) {
  loop_emission_counter_++;

  std::string guard = namer_.NameFor("tint_msl_is_first_" +
                                     std::to_string(loop_emission_counter_));

  if (stmt->has_continuing()) {
    make_indent();

    // Continuing variables get their own scope.
    out_ << "{" << std::endl;
    increment_indent();

    make_indent();
    out_ << "bool " << guard << " = true;" << std::endl;
  }

  make_indent();
  out_ << "for(;;) {" << std::endl;
  increment_indent();

  if (stmt->has_continuing()) {
    make_indent();
    out_ << "if (!" << guard << ")";

    if (!EmitStatementBlockAndNewline(stmt->continuing())) {
      return false;
    }

    make_indent();
    out_ << guard << " = false;" << std::endl;
    out_ << std::endl;
  }

  for (const auto& s : stmt->body()) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  // Close the scope for any continuing variables.
  if (stmt->has_continuing()) {
    decrement_indent();
    make_indent();
    out_ << "}" << std::endl;
  }

  return true;
}

bool GeneratorImpl::EmitKill(ast::KillStatement*) {
  make_indent();
  // TODO(dsinclair): Verify this is correct when the kill semantics are defined
  // for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  out_ << "discard_fragment();" << std::endl;
  return true;
}

bool GeneratorImpl::EmitElse(ast::ElseStatement* stmt) {
  if (stmt->HasCondition()) {
    out_ << " else if (";
    if (!EmitExpression(stmt->condition())) {
      return false;
    }
    out_ << ")";
  } else {
    out_ << " else";
  }

  return EmitStatementBlock(stmt->body());
}

bool GeneratorImpl::EmitIf(ast::IfStatement* stmt) {
  make_indent();

  out_ << "if (";
  if (!EmitExpression(stmt->condition())) {
    return false;
  }
  out_ << ")";

  if (!EmitStatementBlock(stmt->body())) {
    return false;
  }

  for (const auto& e : stmt->else_statements()) {
    if (!EmitElse(e.get())) {
      return false;
    }
  }
  out_ << std::endl;

  return true;
}

bool GeneratorImpl::EmitMemberAccessor(ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(expr->structure())) {
    return false;
  }

  out_ << ".";

  return EmitExpression(expr->member());
}

bool GeneratorImpl::EmitReturn(ast::ReturnStatement* stmt) {
  make_indent();

  out_ << "return";

  if (generating_entry_point_) {
    auto out_data = ep_name_to_out_data_.find(current_ep_name_);
    if (out_data != ep_name_to_out_data_.end()) {
      out_ << " " << out_data->second.var_name;
    }
  } else if (stmt->has_value()) {
    out_ << " ";
    if (!EmitExpression(stmt->value())) {
      return false;
    }
  }
  out_ << ";" << std::endl;
  return true;
}

bool GeneratorImpl::EmitStatementBlock(const ast::StatementList& statements) {
  out_ << " {" << std::endl;

  increment_indent();

  for (const auto& s : statements) {
    if (!EmitStatement(s.get())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}";

  return true;
}

bool GeneratorImpl::EmitStatementBlockAndNewline(
    const ast::StatementList& statements) {
  const bool result = EmitStatementBlock(statements);
  if (result) {
    out_ << std::endl;
  }
  return result;
}

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    return EmitAssign(stmt->AsAssign());
  }
  if (stmt->IsBreak()) {
    return EmitBreak(stmt->AsBreak());
  }
  if (stmt->IsContinue()) {
    return EmitContinue(stmt->AsContinue());
  }
  if (stmt->IsFallthrough()) {
    make_indent();
    out_ << "/* fallthrough */" << std::endl;
    return true;
  }
  if (stmt->IsIf()) {
    return EmitIf(stmt->AsIf());
  }
  if (stmt->IsKill()) {
    return EmitKill(stmt->AsKill());
  }
  if (stmt->IsLoop()) {
    return EmitLoop(stmt->AsLoop());
  }
  if (stmt->IsReturn()) {
    return EmitReturn(stmt->AsReturn());
  }
  if (stmt->IsSwitch()) {
    return EmitSwitch(stmt->AsSwitch());
  }
  if (stmt->IsVariableDecl()) {
    return EmitVariable(stmt->AsVariableDecl()->variable());
  }

  error_ = "unknown statement type: " + stmt->str();
  return false;
}

bool GeneratorImpl::EmitSwitch(ast::SwitchStatement* stmt) {
  make_indent();

  out_ << "switch(";
  if (!EmitExpression(stmt->condition())) {
    return false;
  }
  out_ << ") {" << std::endl;

  increment_indent();

  for (const auto& s : stmt->body()) {
    if (!EmitCase(s.get())) {
      return false;
    }
  }

  decrement_indent();
  make_indent();
  out_ << "}" << std::endl;

  return true;
}

bool GeneratorImpl::EmitType(ast::type::Type* type, const std::string& name) {
  if (type->IsAlias()) {
    auto* alias = type->AsAlias();
    out_ << namer_.NameFor(alias->name());
  } else if (type->IsArray()) {
    auto* ary = type->AsArray();

    ast::type::Type* base_type = ary;
    std::vector<uint32_t> sizes;
    while (base_type->IsArray()) {
      if (base_type->AsArray()->IsRuntimeArray()) {
        sizes.push_back(1);
      } else {
        sizes.push_back(base_type->AsArray()->size());
      }
      base_type = base_type->AsArray()->type();
    }
    if (!EmitType(base_type, "")) {
      return false;
    }
    if (!name.empty()) {
      out_ << " " << namer_.NameFor(name);
    }
    for (uint32_t size : sizes) {
      out_ << "[" << size << "]";
    }
  } else if (type->IsBool()) {
    out_ << "bool";
  } else if (type->IsF32()) {
    out_ << "float";
  } else if (type->IsI32()) {
    out_ << "int";
  } else if (type->IsMatrix()) {
    auto* mat = type->AsMatrix();
    if (!EmitType(mat->type(), "")) {
      return false;
    }
    out_ << mat->columns() << "x" << mat->rows();
  } else if (type->IsPointer()) {
    auto* ptr = type->AsPointer();
    // TODO(dsinclair): Storage class?
    if (!EmitType(ptr->type(), "")) {
      return false;
    }
    out_ << "*";
  } else if (type->IsStruct()) {
    auto* str = type->AsStruct()->impl();
    // TODO(dsinclair): Block decoration?
    // if (str->decoration() != ast::StructDecoration::kNone) {
    // }
    out_ << "struct {" << std::endl;

    increment_indent();
    for (const auto& mem : str->members()) {
      make_indent();
      // TODO(dsinclair): Member decorations?
      // if (!mem->decorations().empty()) {
      // }

      if (!EmitType(mem->type(), mem->name())) {
        return false;
      }
      // Array member name will be output with the type
      if (!mem->type()->IsArray()) {
        out_ << " " << namer_.NameFor(mem->name());
      }
      out_ << ";" << std::endl;
    }
    decrement_indent();
    make_indent();

    out_ << "}";
  } else if (type->IsU32()) {
    out_ << "uint";
  } else if (type->IsVector()) {
    auto* vec = type->AsVector();
    if (!EmitType(vec->type(), "")) {
      return false;
    }
    out_ << vec->size();
  } else if (type->IsVoid()) {
    out_ << "void";
  } else {
    error_ = "unknown type in EmitType";
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitUnaryOp(ast::UnaryOpExpression* expr) {
  switch (expr->op()) {
    case ast::UnaryOp::kNot:
      out_ << "!";
      break;
    case ast::UnaryOp::kNegation:
      out_ << "-";
      break;
  }
  out_ << "(";

  if (!EmitExpression(expr->expr())) {
    return false;
  }

  out_ << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(ast::Variable* var) {
  make_indent();

  // TODO(dsinclair): Handle variable decorations
  if (var->IsDecorated()) {
    error_ = "Variable decorations are not handled yet";
    return false;
  }

  if (var->is_const()) {
    out_ << "const ";
  }
  if (!EmitType(var->type(), var->name())) {
    return false;
  }
  if (!var->type()->IsArray()) {
    out_ << " " << var->name();
  }

  if (var->constructor() != nullptr) {
    out_ << " = ";
    if (!EmitExpression(var->constructor())) {
      return false;
    }
  }
  out_ << ";" << std::endl;

  return true;
}

}  // namespace msl
}  // namespace writer
}  // namespace tint
