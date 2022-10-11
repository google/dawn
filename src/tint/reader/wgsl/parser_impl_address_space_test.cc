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

struct AddressSpaceData {
    const char* input;
    ast::AddressSpace result;
};
inline std::ostream& operator<<(std::ostream& out, AddressSpaceData data) {
    out << std::string(data.input);
    return out;
}

class ParserAddressSpaceTest : public ParserImplTestWithParam<AddressSpaceData> {};

TEST_P(ParserAddressSpaceTest, Parses) {
    auto params = GetParam();
    auto p = parser(params.input);

    auto sc = p->expect_address_space("test");
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(p->has_error());
    EXPECT_EQ(sc.value, params.result);

    auto& t = p->next();
    EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    ParserAddressSpaceTest,
    testing::Values(AddressSpaceData{"uniform", ast::AddressSpace::kUniform},
                    AddressSpaceData{"workgroup", ast::AddressSpace::kWorkgroup},
                    AddressSpaceData{"storage", ast::AddressSpace::kStorage},
                    AddressSpaceData{"private", ast::AddressSpace::kPrivate},
                    AddressSpaceData{"function", ast::AddressSpace::kFunction}));

TEST_F(ParserImplTest, AddressSpace_NoMatch) {
    auto p = parser("not-a-address-space");
    auto sc = p->expect_address_space("test");
    EXPECT_EQ(sc.errored, true);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:1: expected address space for test
Possible values: 'function', 'private', 'push_constant', 'storage', 'uniform', 'workgroup')");
}

}  // namespace
}  // namespace tint::reader::wgsl
