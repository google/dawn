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

struct VariableStorageData {
    const char* input;
    ast::StorageClass storage_class;
    ast::Access access;
};
inline std::ostream& operator<<(std::ostream& out, VariableStorageData data) {
    out << std::string(data.input);
    return out;
}

class VariableQualifierTest : public ParserImplTestWithParam<VariableStorageData> {};

TEST_P(VariableQualifierTest, ParsesStorageClass) {
    auto params = GetParam();
    auto p = parser(std::string("<") + params.input + ">");

    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_TRUE(sc.matched);
    EXPECT_EQ(sc->storage_class, params.storage_class);
    EXPECT_EQ(sc->access, params.access);

    auto t = p->next();
    EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    VariableQualifierTest,
    testing::Values(
        VariableStorageData{"uniform", ast::StorageClass::kUniform, ast::Access::kUndefined},
        VariableStorageData{"workgroup", ast::StorageClass::kWorkgroup, ast::Access::kUndefined},
        VariableStorageData{"storage", ast::StorageClass::kStorage, ast::Access::kUndefined},
        VariableStorageData{"storage_buffer", ast::StorageClass::kStorage, ast::Access::kUndefined},
        VariableStorageData{"private", ast::StorageClass::kPrivate, ast::Access::kUndefined},
        VariableStorageData{"function", ast::StorageClass::kFunction, ast::Access::kUndefined},
        VariableStorageData{"storage, read", ast::StorageClass::kStorage, ast::Access::kRead},
        VariableStorageData{"storage, write", ast::StorageClass::kStorage, ast::Access::kWrite},
        VariableStorageData{"storage, read_write", ast::StorageClass::kStorage,
                            ast::Access::kReadWrite}));

TEST_F(ParserImplTest, VariableQualifier_NoMatch) {
    auto p = parser("<not-a-storage-class>");
    auto sc = p->variable_qualifier();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), "1:2: invalid storage class for variable declaration");
}

TEST_F(ParserImplTest, VariableQualifier_Empty) {
    auto p = parser("<>");
    auto sc = p->variable_qualifier();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), "1:2: invalid storage class for variable declaration");
}

TEST_F(ParserImplTest, VariableQualifier_MissingLessThan) {
    auto p = parser("private>");
    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(sc.matched);

    auto t = p->next();
    ASSERT_TRUE(t.Is(Token::Type::kPrivate));
}

TEST_F(ParserImplTest, VariableQualifier_MissingLessThan_AfterSC) {
    auto p = parser("private, >");
    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(sc.matched);

    auto t = p->next();
    ASSERT_TRUE(t.Is(Token::Type::kPrivate));
}

TEST_F(ParserImplTest, VariableQualifier_MissingGreaterThan) {
    auto p = parser("<private");
    auto sc = p->variable_qualifier();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), "1:9: expected '>' for variable declaration");
}

}  // namespace
}  // namespace tint::reader::wgsl
