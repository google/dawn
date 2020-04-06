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

#include "src/type_determiner.h"

namespace tint {

TypeDeterminer::TypeDeterminer(Context* ctx) : ctx_(*ctx) {
  // TODO(dsinclair): Temporary usage to avoid compiler warning
  static_cast<void>(ctx_.type_mgr());
}

TypeDeterminer::~TypeDeterminer() = default;

bool TypeDeterminer::Determine(ast::Module*) {
  return true;
}

}  // namespace tint
