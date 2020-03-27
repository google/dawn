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

#include "src/reader/spirv/parser.h"

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "src/context.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ParserTest = testing::Test;

TEST_F(ParserTest, Uint32VecEmpty) {
  std::vector<uint32_t> data;
  Context ctx;
  Parser p(&ctx, data);
  EXPECT_FALSE(p.Parse());
  // TODO(dneto): What message?
}

// TODO(dneto): uint32 vec, valid SPIR-V
// TODO(dneto): uint32 vec, invalid SPIR-V

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
