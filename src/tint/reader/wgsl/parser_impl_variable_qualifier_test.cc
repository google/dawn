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

#include "src/tint/ast/test_helper.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

struct VariableStorageData {
    const char* input;
    builtin::AddressSpace address_space;
    builtin::Access access;
};
inline std::ostream& operator<<(std::ostream& out, VariableStorageData data) {
    out << std::string(data.input);
    return out;
}

class VariableQualifierTest : public ParserImplTestWithParam<VariableStorageData> {};

TEST_P(VariableQualifierTest, ParsesAddressSpace) {
    auto params = GetParam();
    auto p = parser(std::string("var<") + params.input + "> name");

    auto sc = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_TRUE(sc.matched);
    if (params.address_space != builtin::AddressSpace::kUndefined) {
        ast::CheckIdentifier(sc->address_space, utils::ToString(params.address_space));
    } else {
        EXPECT_EQ(sc->address_space, nullptr);
    }
    if (params.access != builtin::Access::kUndefined) {
        ast::CheckIdentifier(sc->access, utils::ToString(params.access));
    } else {
        EXPECT_EQ(sc->access, nullptr);
    }

    auto& t = p->next();
    EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    VariableQualifierTest,
    testing::Values(VariableStorageData{"uniform", builtin::AddressSpace::kUniform,
                                        builtin::Access::kUndefined},
                    VariableStorageData{"workgroup", builtin::AddressSpace::kWorkgroup,
                                        builtin::Access::kUndefined},
                    VariableStorageData{"storage", builtin::AddressSpace::kStorage,
                                        builtin::Access::kUndefined},
                    VariableStorageData{"private", builtin::AddressSpace::kPrivate,
                                        builtin::Access::kUndefined},
                    VariableStorageData{"function", builtin::AddressSpace::kFunction,
                                        builtin::Access::kUndefined},
                    VariableStorageData{"storage, read", builtin::AddressSpace::kStorage,
                                        builtin::Access::kRead},
                    VariableStorageData{"storage, write", builtin::AddressSpace::kStorage,
                                        builtin::Access::kWrite},
                    VariableStorageData{"storage, read_write", builtin::AddressSpace::kStorage,
                                        builtin::Access::kReadWrite}));

TEST_F(ParserImplTest, VariableQualifier_Empty) {
    auto p = parser("var<> name");
    auto sc = p->variable_decl();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), R"(1:5: expected expression for 'var' address space)");
}

TEST_F(ParserImplTest, VariableQualifier_MissingLessThan) {
    auto p = parser("private>");
    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(sc.matched);

    auto& t = p->next();
    ASSERT_TRUE(t.Is(Token::Type::kIdentifier));
}

TEST_F(ParserImplTest, VariableQualifier_MissingLessThan_AfterSC) {
    auto p = parser("private, >");
    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(sc.matched);

    auto& t = p->next();
    ASSERT_TRUE(t.Is(Token::Type::kIdentifier));
}

TEST_F(ParserImplTest, VariableQualifier_MissingGreaterThan) {
    auto p = parser("<private");
    auto sc = p->variable_qualifier();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), "1:1: missing closing '>' for variable declaration");
}

}  // namespace
}  // namespace tint::reader::wgsl
