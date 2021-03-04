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
#include <utility>

#include "src/program_builder.h"

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

  Data::Remappings remappings;

  switch (cfg_.method) {
    case Method::kMonotonic:
      ctx.ReplaceAll([&](Symbol sym) {
        auto str_in = in->Symbols().NameFor(sym);
        auto str_out = "_tint_" + std::to_string(sym.value());
        remappings.emplace(str_in, str_out);
        return out.Symbols().Register(str_out);
      });
      break;
  }
  ctx.Clone();

  return Output(Program(std::move(out)),
                std::make_unique<Data>(std::move(remappings)));
}

}  // namespace transform
}  // namespace tint
