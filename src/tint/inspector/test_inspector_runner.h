// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_INSPECTOR_TEST_INSPECTOR_RUNNER_H_
#define SRC_TINT_INSPECTOR_TEST_INSPECTOR_RUNNER_H_

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "tint/tint.h"

namespace tint::inspector {

/// Utility class for running shaders in inspector tests
class InspectorRunner {
  public:
    InspectorRunner();
    virtual ~InspectorRunner();

    /// Create a Program with Inspector from the provided WGSL shader.
    /// Should only be called once per test.
    /// @param shader a WGSL shader
    /// @returns a reference to the Inspector for the built Program.
    Inspector& Initialize(std::string shader);

  protected:
    /// File created from input shader and used to create Program.
    std::unique_ptr<Source::File> file_;
    /// Program created by this runner.
    std::unique_ptr<Program> program_;
    /// Inspector for |program_|
    std::unique_ptr<Inspector> inspector_;
};

}  // namespace tint::inspector

#endif  // SRC_TINT_INSPECTOR_TEST_INSPECTOR_RUNNER_H_
