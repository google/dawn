// Copyright 2017 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

class InputStateTest : public ValidationTest {
    protected:
      void CreatePipeline(bool success,
                          const utils::ComboInputStateDescriptor& state,
                          std::string vertexSource) {
          dawn::ShaderModule vsModule =
              utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, vertexSource.c_str());
          dawn::ShaderModule fsModule =
              utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
                }
            )");

          utils::ComboRenderPipelineDescriptor descriptor(device);
          descriptor.cVertexStage.module = vsModule;
          descriptor.cFragmentStage.module = fsModule;
          descriptor.inputState = &state;
          descriptor.cColorStates[0]->format = dawn::TextureFormat::R8G8B8A8Unorm;

          if (!success) {
              ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
          } else {
              device.CreateRenderPipeline(&descriptor);
          }
      }
};

// Check an empty input state is valid
TEST_F(InputStateTest, EmptyIsOk) {
    utils::ComboInputStateDescriptor state;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check validation that pipeline vertex inputs are backed by attributes in the input state
TEST_F(InputStateTest, PipelineCompatibility) {
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.cInputs[0].stride = 2 * sizeof(float);
    state.numAttributes = 2;
    state.cAttributes[1].shaderLocation = 1;
    state.cAttributes[1].offset = sizeof(float);

    // Control case: pipeline with one input per attribute
    CreatePipeline(true, state, R"(
        #version 450
        layout(location = 0) in vec4 a;
        layout(location = 1) in vec4 b;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Check it is valid for the pipeline to use a subset of the InputState
    CreatePipeline(true, state, R"(
        #version 450
        layout(location = 0) in vec4 a;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Check for an error when the pipeline uses an attribute not in the input state
    CreatePipeline(false, state, R"(
        #version 450
        layout(location = 2) in vec4 a;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Test that a stride of 0 is valid
TEST_F(InputStateTest, StrideZero) {
    // Works ok without attributes
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Works ok with attributes at a large-ish offset
    state.numAttributes = 1;
    state.cAttributes[0].offset = 128;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Test that we cannot set an already set input
TEST_F(InputStateTest, AlreadySetInput) {
    // Control case
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Oh no, input 0 is set twice
    state.numInputs = 2;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check out of bounds condition on input slot
TEST_F(InputStateTest, SetInputSlotOutOfBounds) {
    // Control case, setting last input slot
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.cInputs[0].inputSlot = kMaxVertexInputs - 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test input slot OOB
    state.cInputs[0].inputSlot = kMaxVertexInputs;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check out of bounds condition on input stride
TEST_F(InputStateTest, SetInputStrideOutOfBounds) {
    // Control case, setting max input stride
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.cInputs[0].stride = kMaxVertexInputStride;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test input stride OOB
    state.cInputs[0].stride = kMaxVertexInputStride + 1;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Test that we cannot set an already set attribute
TEST_F(InputStateTest, AlreadySetAttribute) {
    // Control case, setting last attribute
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.numAttributes = 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Oh no, attribute 0 is set twice
    state.numAttributes = 2;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check out of bounds condition on attribute shader location
TEST_F(InputStateTest, SetAttributeLocationOutOfBounds) {
    // Control case, setting last attribute shader location
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.numAttributes = 1;
    state.cAttributes[0].shaderLocation = kMaxVertexAttributes - 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test attribute location OOB
    state.cAttributes[0].shaderLocation = kMaxVertexAttributes;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check attribute offset out of bounds
TEST_F(InputStateTest, SetAttributeOffsetOutOfBounds) {
    // Control case, setting max attribute offset for FloatR32 vertex format
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.numAttributes = 1;
    state.cAttributes[0].offset = kMaxVertexAttributeEnd - sizeof(dawn::VertexFormat::Float);
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test attribute offset out of bounds
    state.cAttributes[0].offset = kMaxVertexAttributeEnd - 1;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check attribute offset overflow
TEST_F(InputStateTest, SetAttributeOffsetOverflow) {
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.numAttributes = 1;
    state.cAttributes[0].offset = std::numeric_limits<uint32_t>::max();
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check that all attributes must be backed by an input
TEST_F(InputStateTest, RequireInputForAttribute) {
    // Control case
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.numAttributes = 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Attribute 0 uses input 1 which doesn't exist
    state.cAttributes[0].inputSlot = 1;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check OOB checks for an attribute's input
TEST_F(InputStateTest, SetAttributeOOBCheckForInputs) {
    // Control case
    utils::ComboInputStateDescriptor state;
    state.numInputs = 1;
    state.numAttributes = 1;
    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Could crash if we didn't check for OOB
    state.cAttributes[0].inputSlot = 1000000;
    CreatePipeline(false, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}
