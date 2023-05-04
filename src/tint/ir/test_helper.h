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

#ifndef SRC_TINT_IR_TEST_HELPER_H_
#define SRC_TINT_IR_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/ir/builder_impl.h"
#include "src/tint/ir/disassembler.h"
#include "src/tint/number.h"
#include "src/tint/program_builder.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
  public:
    TestHelperBase() = default;

    ~TestHelperBase() override = default;

    /// Builds and returns a BuilderImpl from the program.
    /// @note The builder is only created once. Multiple calls to Build() will
    /// return the same builder without rebuilding.
    /// @return the builder
    BuilderImpl& CreateBuilder() {
        SetResolveOnBuild(true);

        if (gen_) {
            return *gen_;
        }
        diag::Formatter formatter;

        program_ = std::make_unique<Program>(std::move(*this));
        [&]() { ASSERT_TRUE(program_->IsValid()) << formatter.format(program_->Diagnostics()); }();
        gen_ = std::make_unique<BuilderImpl>(program_.get());
        return *gen_;
    }

    /// Injects a flow block into the builder
    /// @returns the injected block
    ir::Block* InjectFlowBlock() {
        auto* block = gen_->builder.CreateBlock();
        gen_->current_flow_block = block;
        return block;
    }

    /// Creates a BuilderImpl without an originating program. This is used for testing the
    /// expressions which don't require the full builder implementation. The current flow block
    /// is initialized with an empty block.
    /// @returns the BuilderImpl for testing.
    BuilderImpl& CreateEmptyBuilder() {
        program_ = std::make_unique<Program>();
        gen_ = std::make_unique<BuilderImpl>(program_.get());
        gen_->current_flow_block = gen_->builder.CreateBlock();
        return *gen_;
    }

    /// Build the module, cleaning up the program before returning.
    /// @returns the generated module
    utils::Result<Module> Build() {
        auto& b = CreateBuilder();
        auto m = b.Build();

        // Store the error away in case we need it
        error_ = b.Diagnostics().str();

        // Explicitly remove program to guard against pointers back to ast. Note, this does mean the
        // BuilderImpl is pointing to an invalid program. We keep the BuilderImpl around because we
        // need to be able to map from ast pointers to flow nodes in tests.
        program_ = nullptr;
        return m;
    }

    /// @param node the ast node to lookup
    /// @returns the IR flow node for the given ast node.
    const ir::FlowNode* FlowNodeForAstNode(const ast::Node* node) const {
        return gen_->FlowNodeForAstNode(node);
    }

    /// @param mod the module
    /// @returns the disassembly string of the module
    std::string Disassemble(Module& mod) const {
        Disassembler d(mod);
        return d.Disassemble();
    }

    /// @returns the error generated during build, if any
    std::string Error() const { return error_; }

  private:
    std::unique_ptr<BuilderImpl> gen_;

    /// The program built with a call to Build()
    std::unique_ptr<Program> program_;

    /// Error generated when calling `Build`
    std::string error_;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::ir

#endif  // SRC_TINT_IR_TEST_HELPER_H_
