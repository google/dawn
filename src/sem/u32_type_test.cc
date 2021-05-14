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

#include "src/sem/test_helper.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {
namespace {

using U32Test = TestHelper;

TEST_F(U32Test, TypeName) {
  U32 u;
  EXPECT_EQ(u.type_name(), "__u32");
}

TEST_F(U32Test, FriendlyName) {
  U32 u;
  EXPECT_EQ(u.FriendlyName(Symbols()), "u32");
}

}  // namespace
}  // namespace sem
}  // namespace tint
