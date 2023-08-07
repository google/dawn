// Copyright 2022 The Tint Authors.
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

#include "src/tint/lang/wgsl/helpers/check_supported_extensions.h"

#include "gtest/gtest.h"

#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::writer {
namespace {

class CheckSupportedExtensionsTest : public ::testing::Test, public ProgramBuilder {};

TEST_F(CheckSupportedExtensionsTest, Supported) {
    Enable(core::Extension::kF16);

    ASSERT_TRUE(CheckSupportedExtensions("writer", AST(), Diagnostics(),
                                         Vector{
                                             core::Extension::kF16,
                                             core::Extension::kChromiumExperimentalDp4A,
                                         }));
}

TEST_F(CheckSupportedExtensionsTest, Unsupported) {
    Enable(Source{{12, 34}}, core::Extension::kF16);

    ASSERT_FALSE(CheckSupportedExtensions("writer", AST(), Diagnostics(),
                                          Vector{
                                              core::Extension::kChromiumExperimentalDp4A,
                                          }));
    EXPECT_EQ(Diagnostics().str(), "12:34 error: writer backend does not support extension 'f16'");
}

}  // namespace
}  // namespace tint::writer
