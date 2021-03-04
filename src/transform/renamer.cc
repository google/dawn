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

#include "src/transform/renamer.h"

#include <memory>
#include <unordered_set>
#include <utility>

#include "src/ast/member_accessor_expression.h"
#include "src/program_builder.h"
#include "src/semantic/call.h"
#include "src/semantic/intrinsic.h"
#include "src/semantic/member_accessor_expression.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Renamer::Data);

namespace tint {
namespace transform {

Renamer::Data::Data(Remappings&& r) : remappings(std::move(r)) {}

Renamer::Data::Data(const Data&) = default;

Renamer::Data::~Data() = default;

Renamer::Renamer() : cfg_{} {}

Renamer::Renamer(const Config& config) : cfg_(config) {}

Renamer::~Renamer() = default;

Transform::Output Renamer::Run(const Program* in) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  // Swizzles and intrinsic calls need to keep their symbols preserved.
  std::unordered_set<ast::IdentifierExpression*> preserve;
  for (auto* node : in->ASTNodes().Objects()) {
    if (auto* member = node->As<ast::MemberAccessorExpression>()) {
      auto* sem = in->Sem().Get(member);
      if (!sem) {
        TINT_ICE(out.Diagnostics())
            << "MemberAccessorExpression has no semantic info";
        continue;
      }
      if (sem->IsSwizzle()) {
        preserve.emplace(member->member());
      }
    } else if (auto* call = node->As<ast::CallExpression>()) {
      auto* sem = in->Sem().Get(call);
      if (!sem) {
        TINT_ICE(out.Diagnostics()) << "CallExpression has no semantic info";
        continue;
      }
      if (sem->Target()->Is<semantic::Intrinsic>()) {
        preserve.emplace(call->func()->As<ast::IdentifierExpression>());
      }
    }
  }

  Data::Remappings remappings;

  switch (cfg_.method) {
    case Method::kMonotonic:
      ctx.ReplaceAll([&](Symbol sym) {
        auto str_in = in->Symbols().NameFor(sym);
        auto it = remappings.find(str_in);
        if (it != remappings.end()) {
          return out.Symbols().Get(it->second);
        }
        auto str_out = "_tint_" + std::to_string(remappings.size() + 1);
        remappings.emplace(str_in, str_out);
        return out.Symbols().Register(str_out);
      });

      ctx.ReplaceAll(
          [&](ast::IdentifierExpression* ident) -> ast::IdentifierExpression* {
            if (preserve.count(ident)) {
              auto sym_in = ident->symbol();
              auto str = in->Symbols().NameFor(sym_in);
              auto sym_out = out.Symbols().Register(str);
              return ctx.dst->create<ast::IdentifierExpression>(
                  ctx.Clone(ident->source()), sym_out);
            }
            return nullptr;  // Clone ident. Uses the symbol remapping above.
          });
      break;
  }
  ctx.Clone();

  return Output(Program(std::move(out)),
                std::make_unique<Data>(std::move(remappings)));
}

}  // namespace transform
}  // namespace tint
