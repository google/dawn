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

#include "src/writer/hlsl/generator_impl.h"

#include "src/ast/binary_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/case_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/sint_literal.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

bool last_is_break_or_fallthrough(const ast::StatementList& stmts) {
  if (stmts.empty()) {
    return false;
  }

  return stmts.back()->IsBreak() || stmts.back()->IsFallthrough();
}

}  // namespace

GeneratorImpl::GeneratorImpl(ast::Module* module) : module_(module) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  for (const auto& global : module_->global_variables()) {
    global_variables_.set(global->name(), global.get());
  }
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
      // TODO(dsinclair): Implement support ...
      error_ = "&& not supported yet";
      return false;
    case ast::BinaryOp::kLogicalOr:
      // TODO(dsinclair): Implement support ...
      error_ = "|| not supported yet";
      return false;
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

bool GeneratorImpl::EmitContinue(ast::ContinueStatement*) {
  make_indent();
  out_ << "continue;" << std::endl;
  return true;
}

bool GeneratorImpl::EmitExpression(ast::Expression* expr) {
  if (expr->IsBinary()) {
    return EmitBinary(expr->AsBinary());
  }
  if (expr->IsIdentifier()) {
    return EmitIdentifier(expr->AsIdentifier());
  }
  if (expr->IsUnaryOp()) {
    return EmitUnaryOp(expr->AsUnaryOp());
  }

  error_ = "unknown expression type: " + expr->str();
  return false;
}

bool GeneratorImpl::global_is_in_struct(ast::Variable*) const {
  return false;
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
    if (global_is_in_struct(var)) {
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

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
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
  if (stmt->IsReturn()) {
    return EmitReturn(stmt->AsReturn());
  }

  error_ = "unknown statement type: " + stmt->str();
  return false;
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

}  // namespace hlsl
}  // namespace writer
}  // namespace tint
