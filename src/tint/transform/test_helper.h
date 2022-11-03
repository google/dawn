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

#ifndef SRC_TINT_TRANSFORM_TEST_HELPER_H_
#define SRC_TINT_TRANSFORM_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/transform/manager.h"
#include "src/tint/writer/wgsl/generator.h"

namespace tint::transform {

/// @param program the program to get an output WGSL string from
/// @returns the output program as a WGSL string, or an error string if the
/// program is not valid.
inline std::string str(const Program& program) {
    diag::Formatter::Style style;
    style.print_newline_at_end = false;

    if (!program.IsValid()) {
        return diag::Formatter(style).format(program.Diagnostics());
    }

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    if (!result.success) {
        return "WGSL writer failed:\n" + result.error;
    }

    auto res = result.wgsl;
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

/// Helper class for testing transforms
template <typename BASE>
class TransformTestBase : public BASE {
  public:
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
    template <typename... TRANSFORMS>
    Output Run(std::string in, const DataMap& data = {}) {
        auto file = std::make_unique<Source::File>("test", in);
        auto program = reader::wgsl::Parse(file.get());

        // Keep this pointer alive after Transform() returns
        files_.emplace_back(std::move(file));

        return Run<TRANSFORMS...>(std::move(program), data);
    }

    /// Transforms and returns program `program`, transformed using a transform of
    /// type `TRANSFORM`.
    /// @param program the input Program
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return the transformed output
    template <typename... TRANSFORMS>
    Output Run(Program&& program, const DataMap& data = {}) {
        if (!program.IsValid()) {
            return Output(std::move(program));
        }

        Manager manager;
        for (auto* transform_ptr : std::initializer_list<Transform*>{new TRANSFORMS()...}) {
            manager.append(std::unique_ptr<Transform>(transform_ptr));
        }
        return manager.Run(&program, data);
    }

    /// @param program the input program
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return true if the transform should be run for the given input.
    template <typename TRANSFORM>
    bool ShouldRun(Program&& program, const DataMap& data = {}) {
        if (!program.IsValid()) {
            ADD_FAILURE() << "ShouldRun() called with invalid program: "
                          << program.Diagnostics().str();
            return false;
        }

        const Transform& t = TRANSFORM();

        DataMap outputs;
        auto result = t.Apply(&program, data, outputs);
        if (!result) {
            return false;
        }
        if (!result->IsValid()) {
            ADD_FAILURE() << "Apply() called by ShouldRun() returned errors: "
                          << result->Diagnostics().str();
            return true;
        }
        return result.has_value();
    }

    /// @param in the input WGSL source
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return true if the transform should be run for the given input.
    template <typename TRANSFORM>
    bool ShouldRun(std::string in, const DataMap& data = {}) {
        auto file = std::make_unique<Source::File>("test", in);
        auto program = reader::wgsl::Parse(file.get());
        return ShouldRun<TRANSFORM>(std::move(program), data);
    }

    /// @param output the output of the transform
    /// @returns the output program as a WGSL string, or an error string if the
    /// program is not valid.
    std::string str(const Output& output) { return transform::str(output.program); }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

using TransformTest = TransformTestBase<testing::Test>;

template <typename T>
using TransformTestWithParam = TransformTestBase<testing::TestWithParam<T>>;

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_TEST_HELPER_H_
