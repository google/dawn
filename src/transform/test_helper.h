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

#ifndef SRC_TRANSFORM_TEST_HELPER_H_
#define SRC_TRANSFORM_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/reader/wgsl/parser.h"
#include "src/transform/manager.h"
#include "src/writer/wgsl/generator.h"

namespace tint {
namespace transform {

/// Helper class for testing transforms
template <typename BASE>
class TransformTestBase : public BASE {
 public:
  /// Transforms and returns the WGSL source `in`, transformed using
  /// `transforms`.
  /// @param in the input WGSL source
  /// @param transforms the list of transforms to apply
  /// @param data the optional DataMap to pass to Transform::Run()
  /// @return the transformed output
  Output Run(std::string in,
             std::vector<std::unique_ptr<transform::Transform>> transforms,
             const DataMap& data = {}) {
    auto file = std::make_unique<Source::File>("test", in);
    auto program = reader::wgsl::Parse(file.get());

    // Keep this pointer alive after Transform() returns
    files_.emplace_back(std::move(file));

    if (!program.IsValid()) {
      return Output(std::move(program));
    }

    Manager manager;
    for (auto& transform : transforms) {
      manager.append(std::move(transform));
    }
    return manager.Run(&program, data);
  }

  /// Transforms and returns the WGSL source `in`, transformed using
  /// `transform`.
  /// @param transform the transform to apply
  /// @param in the input WGSL source
  /// @param data the optional DataMap to pass to Transform::Run()
  /// @return the transformed output
  Output Run(std::string in,
             std::unique_ptr<transform::Transform> transform,
             const DataMap& data = {}) {
    std::vector<std::unique_ptr<transform::Transform>> transforms;
    transforms.emplace_back(std::move(transform));
    return Run(std::move(in), std::move(transforms), data);
  }

  /// Transforms and returns the WGSL source `in`, transformed using
  /// a transform of type `TRANSFORM`.
  /// @param in the input WGSL source
  /// @param data the optional DataMap to pass to Transform::Run()
  /// @return the transformed output
  template <typename TRANSFORM>
  Output Run(std::string in, const DataMap& data = {}) {
    return Run(std::move(in), std::make_unique<TRANSFORM>(), data);
  }

  /// @param output the output of the transform
  /// @returns the output program as a WGSL string, or an error string if the
  /// program is not valid.
  std::string str(const Output& output) {
    diag::Formatter::Style style;
    style.print_newline_at_end = false;

    if (!output.program.IsValid()) {
      return diag::Formatter(style).format(output.program.Diagnostics());
    }

    writer::wgsl::Generator generator(&output.program);
    if (!generator.Generate()) {
      return "WGSL writer failed:\n" + generator.error();
    }

    auto res = generator.result();
    if (res.empty()) {
      return res;
    }
    // The WGSL sometimes has two trailing newlines. Strip them
    while (res.back() == '\n') {
      res.pop_back();
    }
    if (res.empty()) {
      return res;
    }
    return "\n" + res + "\n";
  }

 private:
  std::vector<std::unique_ptr<Source::File>> files_;
};

using TransformTest = TransformTestBase<testing::Test>;

template <typename T>
using TransformTestWithParam = TransformTestBase<testing::TestWithParam<T>>;

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_TEST_HELPER_H_
