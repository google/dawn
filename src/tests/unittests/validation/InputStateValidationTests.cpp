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

constexpr static dawn::VertexInputDescriptor kBaseInput = {
    0,                            // inputSlot
    0,                            // stride
    dawn::InputStepMode::Vertex,  // stepMode
};

class InputStateTest : public ValidationTest {
    protected:
        void CreatePipeline(bool success, const dawn::InputState& inputState, std::string vertexSource) {
            dawn::ShaderModule vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, vertexSource.c_str());
            dawn::ShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
                }
            )");

            utils::ComboRenderPipelineDescriptor descriptor(device);
            descriptor.cVertexStage.module = vsModule;
            descriptor.cFragmentStage.module = fsModule;
            descriptor.inputState = inputState;
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
    dawn::InputState state = AssertWillBeSuccess(device.CreateInputStateBuilder())
        .GetResult();

    CreatePipeline(true, state, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Check validation that pipeline vertex inputs are backed by attributes in the input state
TEST_F(InputStateTest, PipelineCompatibility) {
    dawn::VertexAttributeDescriptor attribute1;
    attribute1.shaderLocation = 0;
    attribute1.inputSlot = 0;
    attribute1.offset = 0;
    attribute1.format = dawn::VertexFormat::FloatR32;

    dawn::VertexAttributeDescriptor attribute2;
    attribute2.shaderLocation = 1;
    attribute2.inputSlot = 0;
    attribute2.offset = sizeof(float);
    attribute2.format = dawn::VertexFormat::FloatR32;

    dawn::VertexInputDescriptor input;
    input.inputSlot = 0;
    input.stride = 2 * sizeof(float);
    input.stepMode = dawn::InputStepMode::Vertex;

    dawn::InputState state = AssertWillBeSuccess(device.CreateInputStateBuilder())
                                 .SetInput(&input)
                                 .SetAttribute(&attribute1)
                                 .SetAttribute(&attribute2)
                                 .GetResult();

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
    AssertWillBeSuccess(device.CreateInputStateBuilder()).SetInput(&kBaseInput).GetResult();

    // Works ok with attributes at a large-ish offset
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = 0;
    attribute.inputSlot = 0;
    attribute.offset = 128;
    attribute.format = dawn::VertexFormat::FloatR32;

    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();
}

// Test that we cannot set an already set input
TEST_F(InputStateTest, AlreadySetInput) {
    // Control case
    AssertWillBeSuccess(device.CreateInputStateBuilder()).SetInput(&kBaseInput).GetResult();

    // Oh no, input 0 is set twice
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetInput(&kBaseInput)
        .GetResult();
}

// Check out of bounds condition on input slot
TEST_F(InputStateTest, SetInputSlotOutOfBounds) {
    // Control case, setting last input slot
    dawn::VertexInputDescriptor input;
    input.inputSlot = kMaxVertexInputs - 1;
    input.stride = 0;
    input.stepMode = dawn::InputStepMode::Vertex;

    AssertWillBeSuccess(device.CreateInputStateBuilder()).SetInput(&input).GetResult();

    // Test input slot OOB
    input.inputSlot = kMaxVertexInputs;
    AssertWillBeError(device.CreateInputStateBuilder()).SetInput(&input).GetResult();
}

// Check out of bounds condition on input stride
TEST_F(InputStateTest, SetInputStrideOutOfBounds) {
    // Control case, setting max input stride
    dawn::VertexInputDescriptor input;
    input.inputSlot = 0;
    input.stride = kMaxVertexInputStride;
    input.stepMode = dawn::InputStepMode::Vertex;
    AssertWillBeSuccess(device.CreateInputStateBuilder()).SetInput(&input).GetResult();

    // Test input stride OOB
    input.stride = kMaxVertexInputStride + 1;
    AssertWillBeError(device.CreateInputStateBuilder()).SetInput(&input).GetResult();
}

// Test that we cannot set an already set attribute
TEST_F(InputStateTest, AlreadySetAttribute) {
    // Control case, setting last attribute
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = 0;
    attribute.inputSlot = 0;
    attribute.offset = 0;
    attribute.format = dawn::VertexFormat::FloatR32;

    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();

    // Oh no, attribute 0 is set twice
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .SetAttribute(&attribute)
        .GetResult();
}

// Check out of bounds condition on attribute shader location
TEST_F(InputStateTest, SetAttributeLocationOutOfBounds) {
    // Control case, setting last attribute shader location
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = kMaxVertexAttributes - 1;
    attribute.inputSlot = 0;
    attribute.offset = 0;
    attribute.format = dawn::VertexFormat::FloatR32;

    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();

    // Test attribute location OOB
    attribute.shaderLocation = kMaxVertexAttributes;
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();
}

// Check attribute offset out of bounds
TEST_F(InputStateTest, SetAttributeOffsetOutOfBounds) {
    // Control case, setting max attribute offset for FloatR32 vertex format
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = 0;
    attribute.inputSlot = 0;
    attribute.offset = kMaxVertexAttributeEnd - sizeof(dawn::VertexFormat::FloatR32);
    attribute.format = dawn::VertexFormat::FloatR32;
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();

    // Test attribute offset out of bounds
    attribute.offset = kMaxVertexAttributeEnd - 1;
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();
}

// Check attribute offset overflow
TEST_F(InputStateTest, SetAttributeOffsetOverflow) {
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = 0;
    attribute.inputSlot = 0;
    attribute.offset = std::numeric_limits<uint32_t>::max();
    attribute.format = dawn::VertexFormat::FloatR32;
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();
}

// Check that all attributes must be backed by an input
TEST_F(InputStateTest, RequireInputForAttribute) {
    // Control case
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = 0;
    attribute.inputSlot = 0;
    attribute.offset = 0;
    attribute.format = dawn::VertexFormat::FloatR32;

    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();

    // Attribute 0 uses input 1 which doesn't exist
    attribute.inputSlot = 1;
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();
}

// Check OOB checks for an attribute's input
TEST_F(InputStateTest, SetAttributeOOBCheckForInputs) {
    // Control case
    dawn::VertexAttributeDescriptor attribute;
    attribute.shaderLocation = 0;
    attribute.inputSlot = 0;
    attribute.offset = 0;
    attribute.format = dawn::VertexFormat::FloatR32;

    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();

    // Could crash if we didn't check for OOB
    attribute.inputSlot = 1000000;
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(&kBaseInput)
        .SetAttribute(&attribute)
        .GetResult();
}
