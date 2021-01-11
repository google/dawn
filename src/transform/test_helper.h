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
#include "src/type_determiner.h"
#include "src/writer/wgsl/generator.h"

namespace tint {
namespace transform {

/// Helper class for testing transforms
class TransformTest : public testing::Test {
 public:
  /// Transforms and returns the WGSL source `in`, transformed using
  /// `transforms`.
  /// @param in the input WGSL source
  /// @param transforms the list of transforms to apply
  /// @return the transformed WGSL output
  std::string Transform(
      std::string in,
      std::vector<std::unique_ptr<transform::Transform>> transforms) {
    Source::File file("test", in);
    reader::wgsl::Parser parser(&file);
    if (!parser.Parse()) {
      return "WGSL reader failed:\n" + parser.error();
    }

    auto module = parser.module();
    TypeDeterminer td(&module);
    if (!td.Determine()) {
      return "Type determination failed:\n" + td.error();
    }

    Manager manager;
    for (auto& transform : transforms) {
      manager.append(std::move(transform));
    }
    auto result = manager.Run(&module);

    if (result.diagnostics.contains_errors()) {
      return "manager().Run() errored:\n" +
             diag::Formatter().format(result.diagnostics);
    }

    // Release the source module to ensure there's no uncloned data in result
    { auto tmp = std::move(module); }

    writer::wgsl::Generator generator(&(result.module));
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

  /// Transforms and returns the WGSL source `in`, transformed using
  /// `transform`.
  /// @param transform the transform to apply
  /// @param in the input WGSL source
  /// @return the transformed WGSL output
  std::string Transform(std::string in,
                        std::unique_ptr<transform::Transform> transform) {
    std::vector<std::unique_ptr<transform::Transform>> transforms;
    transforms.emplace_back(std::move(transform));
    return Transform(std::move(in), std::move(transforms));
  }

  /// Transforms and returns the WGSL source `in`, transformed using
  /// a transform of type `TRANSFORM`.
  /// @param in the input WGSL source
  /// @param args the TRANSFORM constructor arguments
  /// @return the transformed WGSL output
  template <typename TRANSFORM, typename... ARGS>
  std::string Transform(std::string in, ARGS&&... args) {
    return Transform(std::move(in),
                     std::make_unique<TRANSFORM>(std::forward<ARGS>(args)...));
  }
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_TEST_HELPER_H_
