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
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct StorageClassData {
  const char* input;
  ast::StorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, StorageClassData data) {
  out << std::string(data.input);
  return out;
}

class StorageClassTest : public testing::TestWithParam<StorageClassData> {
 public:
  StorageClassTest() = default;
  ~StorageClassTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(StorageClassTest, Parses) {
  auto params = GetParam();
  auto* p = parser(params.input);

  auto sc = p->storage_class();
  ASSERT_FALSE(p->has_error());
  EXPECT_EQ(sc, params.result);

  auto t = p->next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    StorageClassTest,
    testing::Values(
        StorageClassData{"in", ast::StorageClass::kInput},
        StorageClassData{"out", ast::StorageClass::kOutput},
        StorageClassData{"uniform", ast::StorageClass::kUniform},
        StorageClassData{"workgroup", ast::StorageClass::kWorkgroup},
        StorageClassData{"uniform_constant",
                         ast::StorageClass::kUniformConstant},
        StorageClassData{"storage_buffer", ast::StorageClass::kStorageBuffer},
        StorageClassData{"image", ast::StorageClass::kImage},
        StorageClassData{"private", ast::StorageClass::kPrivate},
        StorageClassData{"function", ast::StorageClass::kFunction}));

TEST_F(ParserImplTest, StorageClass_NoMatch) {
  auto* p = parser("not-a-storage-class");
  auto sc = p->storage_class();
  ASSERT_EQ(sc, ast::StorageClass::kNone);

  auto t = p->next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.to_str(), "not");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
