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

TEST_F(ParserImplTest, StorageTextureType_Invalid) {
  auto p = parser("abc");
  auto t = p->storage_texture_type();
  EXPECT_FALSE(t.matched);
  EXPECT_FALSE(t.errored);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_1d) {
  auto p = parser("texture_storage_1d");
  auto t = p->storage_texture_type();
  EXPECT_TRUE(t.matched);
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::TextureDimension::k1d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_2d) {
  auto p = parser("texture_storage_2d");
  auto t = p->storage_texture_type();
  EXPECT_TRUE(t.matched);
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_2dArray) {
  auto p = parser("texture_storage_2d_array");
  auto t = p->storage_texture_type();
  EXPECT_TRUE(t.matched);
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::TextureDimension::k2dArray);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, StorageTextureType_3d) {
  auto p = parser("texture_storage_3d");
  auto t = p->storage_texture_type();
  EXPECT_TRUE(t.matched);
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::TextureDimension::k3d);
  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
