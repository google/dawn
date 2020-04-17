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
#include "src/ast/derivative_modifier.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct DerivativeModifierData {
  const char* input;
  ast::DerivativeModifier result;
};
inline std::ostream& operator<<(std::ostream& out,
                                DerivativeModifierData data) {
  out << std::string(data.input);
  return out;
}

class DerivativeModifierTest
    : public testing::TestWithParam<DerivativeModifierData> {
 public:
  DerivativeModifierTest() = default;
  ~DerivativeModifierTest() override = default;

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

TEST_P(DerivativeModifierTest, Parses) {
  auto params = GetParam();
  auto* p = parser(params.input);

  auto mod = p->derivative_modifier();
  ASSERT_FALSE(p->has_error());
  EXPECT_EQ(mod, params.result);

  auto t = p->next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    DerivativeModifierTest,
    testing::Values(
        DerivativeModifierData{"fine", ast::DerivativeModifier::kFine},
        DerivativeModifierData{"coarse", ast::DerivativeModifier::kCoarse}));

TEST_F(ParserImplTest, DerivativeModifier_NoMatch) {
  auto* p = parser("not-a-modifier");
  auto stage = p->derivative_modifier();
  ASSERT_EQ(stage, ast::DerivativeModifier::kNone);

  auto t = p->next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.to_str(), "not");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
