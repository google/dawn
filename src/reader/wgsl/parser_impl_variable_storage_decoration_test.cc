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

#include "gtest/gtest.h"
#include "src/ast/storage_class.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

struct VariableStorageData {
  const char* input;
  ast::StorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, VariableStorageData data) {
  out << std::string(data.input);
  return out;
}
using VariableStorageTest = testing::TestWithParam<VariableStorageData>;
TEST_P(VariableStorageTest, Parses) {
  auto params = GetParam();
  ParserImpl p{std::string("<") + params.input + ">"};

  auto sc = p.variable_storage_decoration();
  ASSERT_FALSE(p.has_error());
  EXPECT_EQ(sc, params.result);

  auto t = p.next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    VariableStorageTest,
    testing::Values(
        VariableStorageData{"in", ast::StorageClass::kInput},
        VariableStorageData{"out", ast::StorageClass::kOutput},
        VariableStorageData{"uniform", ast::StorageClass::kUniform},
        VariableStorageData{"workgroup", ast::StorageClass::kWorkgroup},
        VariableStorageData{"uniform_constant",
                            ast::StorageClass::kUniformConstant},
        VariableStorageData{"storage_buffer",
                            ast::StorageClass::kStorageBuffer},
        VariableStorageData{"image", ast::StorageClass::kImage},
        VariableStorageData{"push_constant", ast::StorageClass::kPushConstant},
        VariableStorageData{"private", ast::StorageClass::kPrivate},
        VariableStorageData{"function", ast::StorageClass::kFunction}));

TEST_F(ParserImplTest, VariableStorageDecoration_NoMatch) {
  ParserImpl p{"<not-a-storage-class>"};
  auto sc = p.variable_storage_decoration();
  ASSERT_EQ(sc, ast::StorageClass::kNone);
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:2: invalid storage class for variable decoration");
}

TEST_F(ParserImplTest, VariableStorageDecoration_Empty) {
  ParserImpl p{"<>"};
  auto sc = p.variable_storage_decoration();
  ASSERT_EQ(sc, ast::StorageClass::kNone);
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:2: invalid storage class for variable decoration");
}

TEST_F(ParserImplTest, VariableStorageDecoration_MissingLessThan) {
  ParserImpl p{"in>"};
  auto sc = p.variable_storage_decoration();
  ASSERT_EQ(sc, ast::StorageClass::kNone);
  ASSERT_FALSE(p.has_error());

  auto t = p.next();
  ASSERT_TRUE(t.IsIn());
}

TEST_F(ParserImplTest, VariableStorageDecoration_MissingGreaterThan) {
  ParserImpl p{"<in"};
  auto sc = p.variable_storage_decoration();
  ASSERT_EQ(sc, ast::StorageClass::kNone);
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(p.error(), "1:4: missing > for variable decoration");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
