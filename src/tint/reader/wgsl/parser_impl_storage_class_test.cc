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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

struct StorageClassData {
    const char* input;
    ast::StorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, StorageClassData data) {
    out << std::string(data.input);
    return out;
}

class StorageClassTest : public ParserImplTestWithParam<StorageClassData> {};

TEST_P(StorageClassTest, Parses) {
    auto params = GetParam();
    auto p = parser(params.input);

    auto sc = p->expect_storage_class("test");
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(p->has_error());
    EXPECT_EQ(sc.value, params.result);

    auto t = p->next();
    EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    StorageClassTest,
    testing::Values(StorageClassData{"uniform", ast::StorageClass::kUniform},
                    StorageClassData{"workgroup", ast::StorageClass::kWorkgroup},
                    StorageClassData{"storage", ast::StorageClass::kStorage},
                    StorageClassData{"storage_buffer", ast::StorageClass::kStorage},
                    StorageClassData{"private", ast::StorageClass::kPrivate},
                    StorageClassData{"function", ast::StorageClass::kFunction}));

TEST_F(ParserImplTest, StorageClass_NoMatch) {
    auto p = parser("not-a-storage-class");
    auto sc = p->expect_storage_class("test");
    EXPECT_EQ(sc.errored, true);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: invalid storage class for test");

    auto t = p->next();
    EXPECT_TRUE(t.IsIdentifier());
    EXPECT_EQ(t.to_str(), "not");
}

}  // namespace
}  // namespace tint::reader::wgsl
