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
#include "src/ast/builtin.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct BuiltinData {
  const char* input;
  ast::Builtin result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
  out << std::string(data.input);
  return out;
}

class BuiltinTest : public testing::TestWithParam<BuiltinData> {
 public:
  BuiltinTest() = default;
  ~BuiltinTest() = default;

  void SetUp() { ctx_.type_mgr = &tm_; }

  void TearDown() {
    impl_ = nullptr;
    ctx_.type_mgr = nullptr;
  }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
  TypeManager tm_;
};

TEST_P(BuiltinTest, Parses) {
  auto params = GetParam();
  auto p = parser(params.input);

  auto builtin = p->builtin_decoration();
  ASSERT_FALSE(p->has_error());
  EXPECT_EQ(builtin, params.result);

  auto t = p->next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    BuiltinTest,
    testing::Values(
        BuiltinData{"position", ast::Builtin::kPosition},
        BuiltinData{"vertex_idx", ast::Builtin::kVertexIdx},
        BuiltinData{"instance_idx", ast::Builtin::kInstanceIdx},
        BuiltinData{"front_facing", ast::Builtin::kFrontFacing},
        BuiltinData{"frag_coord", ast::Builtin::kFragCoord},
        BuiltinData{"frag_depth", ast::Builtin::kFragDepth},
        BuiltinData{"num_workgroups", ast::Builtin::kNumWorkgroups},
        BuiltinData{"workgroup_size", ast::Builtin::kWorkgroupSize},
        BuiltinData{"local_invocation_id", ast::Builtin::kLocalInvocationId},
        BuiltinData{"local_invocation_idx", ast::Builtin::kLocalInvocationIdx},
        BuiltinData{"global_invocation_id",
                    ast::Builtin::kGlobalInvocationId}));

TEST_F(ParserImplTest, BuiltinDecoration_NoMatch) {
  auto p = parser("not-a-builtin");
  auto builtin = p->builtin_decoration();
  ASSERT_EQ(builtin, ast::Builtin::kNone);

  auto t = p->next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.to_str(), "not");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
