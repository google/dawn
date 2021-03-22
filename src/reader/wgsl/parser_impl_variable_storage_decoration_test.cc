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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct VariableStorageData {
  const char* input;
  ast::StorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, VariableStorageData data) {
  out << std::string(data.input);
  return out;
}

class VariableStorageTest
    : public ParserImplTestWithParam<VariableStorageData> {};

TEST_P(VariableStorageTest, Parses) {
  auto params = GetParam();
  auto p = parser(std::string("<") + params.input + ">");

  auto sc = p->variable_storage_decoration();
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(sc.errored);
  EXPECT_TRUE(sc.matched);
  EXPECT_EQ(sc.value, params.result);

  auto t = p->next();
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
        VariableStorageData{"storage", ast::StorageClass::kStorage},
        VariableStorageData{"storage_buffer", ast::StorageClass::kStorage},
        VariableStorageData{"image", ast::StorageClass::kImage},
        VariableStorageData{"private", ast::StorageClass::kPrivate},
        VariableStorageData{"function", ast::StorageClass::kFunction}));

TEST_F(ParserImplTest, VariableStorageDecoration_NoMatch) {
  auto p = parser("<not-a-storage-class>");
  auto sc = p->variable_storage_decoration();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(sc.errored);
  EXPECT_FALSE(sc.matched);
  EXPECT_EQ(p->error(), "1:2: invalid storage class for variable decoration");
}

TEST_F(ParserImplTest, VariableStorageDecoration_Empty) {
  auto p = parser("<>");
  auto sc = p->variable_storage_decoration();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(sc.errored);
  EXPECT_FALSE(sc.matched);
  EXPECT_EQ(p->error(), "1:2: invalid storage class for variable decoration");
}

TEST_F(ParserImplTest, VariableStorageDecoration_MissingLessThan) {
  auto p = parser("in>");
  auto sc = p->variable_storage_decoration();
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(sc.errored);
  EXPECT_FALSE(sc.matched);

  auto t = p->next();
  ASSERT_TRUE(t.IsIn());
}

TEST_F(ParserImplTest, VariableStorageDecoration_MissingGreaterThan) {
  auto p = parser("<in");
  auto sc = p->variable_storage_decoration();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(sc.errored);
  EXPECT_FALSE(sc.matched);
  EXPECT_EQ(p->error(), "1:4: expected '>' for variable decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
