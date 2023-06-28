// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_IR_TRANSFORM_SHADER_IO_H_
#define SRC_TINT_IR_TRANSFORM_SHADER_IO_H_

#include <memory>
#include <utility>

#include "src/tint/ir/builder.h"
#include "src/tint/ir/transform/transform.h"
#include "src/tint/type/manager.h"

namespace tint::ir::transform {

/// ShaderIO is a transform that modifies an entry point function's parameters and return value to
/// prepare them for backend codegen.
class ShaderIO : public utils::Castable<ShaderIO, Transform> {
  public:
    /// Configuration options for the transform.
    struct Config final : public utils::Castable<Config, Data> {
        /// Constructor
        Config();

        /// Copy constructor
        Config(const Config&) = default;

        /// Destructor
        ~Config() override;
    };

    /// Constructor
    ShaderIO();
    /// Destructor
    ~ShaderIO() override;

    /// @copydoc Transform::Run
    void Run(ir::Module* module, const DataMap& inputs, DataMap& outputs) const override;

    /// Abstract base class for the state needed to handle IO for a particular backend target.
    struct BackendState {
        /// Constructor
        /// @param mod the IR module
        /// @param f the entry point function
        BackendState(Module* mod, Function* f) : ir(mod), func(f) {}

        /// Destructor
        virtual ~BackendState();

        /// Add an input.
        /// @param name the name of the input
        /// @param type the type of the input
        /// @param attributes the IO attributes
        virtual void AddInput(Symbol name,
                              const type::Type* type,
                              type::StructMemberAttributes attributes) {
            inputs.Push({name, type, std::move(attributes)});
        }

        /// Add an output.
        /// @param name the name of the output
        /// @param type the type of the output
        /// @param attributes the IO attributes
        virtual void AddOutput(Symbol name,
                               const type::Type* type,
                               type::StructMemberAttributes attributes) {
            outputs.Push({name, type, std::move(attributes)});
        }

        /// Finalize the shader inputs and create any state needed for the new entry point function.
        /// @returns the list of function parameters for the new entry point
        virtual utils::Vector<FunctionParam*, 4> FinalizeInputs() = 0;

        /// Finalize the shader outputs and create state needed for the new entry point function.
        /// @returns the return value for the new entry point
        virtual Value* FinalizeOutputs() = 0;

        /// Get the value of the input at index @p idx
        /// @param builder the IR builder for new instructions
        /// @param idx the index of the input
        /// @returns the value of the input
        virtual Value* GetInput(Builder& builder, uint32_t idx) = 0;

        /// Set the value of the output at index @p idx
        /// @param builder the IR builder for new instructions
        /// @param idx the index of the output
        /// @param value the value to set
        virtual void SetOutput(Builder& builder, uint32_t idx, Value* value) = 0;

      protected:
        /// The IR module.
        Module* ir = nullptr;

        /// The IR builder.
        Builder b{*ir};

        /// The type manager.
        type::Manager& ty{ir->Types()};

        /// The original entry point function.
        Function* func = nullptr;

        /// The list of shader inputs.
        utils::Vector<type::Manager::StructMemberDesc, 4> inputs;

        /// The list of shader outputs.
        utils::Vector<type::Manager::StructMemberDesc, 4> outputs;
    };

  protected:
    struct State;

    /// Create a backend state object.
    /// @param mod the IR module
    /// @param func the entry point function
    /// @returns the backend state object
    virtual std::unique_ptr<ShaderIO::BackendState> MakeBackendState(Module* mod,
                                                                     Function* func) const = 0;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_IR_TRANSFORM_SHADER_IO_H_
