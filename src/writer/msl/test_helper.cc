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

#include "src/writer/msl/test_helper.h"

#include "src/utils/io/command.h"
#include "src/utils/io/tmpfile.h"

namespace tint {
namespace writer {
namespace msl {

namespace {

const char* xcrun_path = nullptr;

}  // namespace

void EnableMSLValidation(const char* xcrun) {
  xcrun_path = xcrun;
}

val::Result Validate(Program* program) {
#ifdef TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
  auto gen = std::make_unique<GeneratorImpl>(program);
  if (!gen->Generate()) {
    return {true, gen->error(), ""};
  }
  return tint::val::MslUsingMetalAPI(gen->result());
#else   // TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
  if (!xcrun_path) {
    return val::Result{};
  }

  auto gen = std::make_unique<GeneratorImpl>(program);
  if (!gen->Generate()) {
    return {true, gen->error(), ""};
  }
  return val::Msl(xcrun_path, gen->result());
#endif  // TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
}

}  // namespace msl
}  // namespace writer
}  // namespace tint
