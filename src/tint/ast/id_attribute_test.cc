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

#include "src/tint/ast/id_attribute.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using IdAttributeTest = TestHelper;

TEST_F(IdAttributeTest, Creation) {
    auto* d = create<IdAttribute>(12);
    EXPECT_EQ(12u, d->value);
}

}  // namespace
}  // namespace tint::ast
