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
        void CreatePipeline(bool success, const dawn::InputState& inputState, std::string vertexSource) {
            DummyRenderPass renderpassData = CreateDummyRenderPass();

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
            descriptor.cColorAttachments[0].format =
                renderpassData.attachmentFormat;

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
    dawn::InputState state = AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 2 * sizeof(float), dawn::InputStepMode::Vertex)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 0)
        .SetAttribute(1, 0, dawn::VertexFormat::FloatR32, sizeof(float))
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
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .GetResult();

    // Works ok with attributes at a large-ish offset
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 128)
        .GetResult();
}

// Test that we cannot set an already set input
TEST_F(InputStateTest, AlreadySetInput) {
    // Control case
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .GetResult();

    // Oh no, input 0 is set twice
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .GetResult();
}

// Check out of bounds condition on SetInput
TEST_F(InputStateTest, SetInputOutOfBounds) {
    // Control case, setting last input
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(kMaxVertexInputs - 1, 0, dawn::InputStepMode::Vertex)
        .GetResult();

    // Test OOB
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(kMaxVertexInputs, 0, dawn::InputStepMode::Vertex)
        .GetResult();
}

// Test that we cannot set an already set attribute
TEST_F(InputStateTest, AlreadySetAttribute) {
    // Control case, setting last attribute
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 0)
        .GetResult();

    // Oh no, attribute 0 is set twice
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 0)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 0)
        .GetResult();
}

// Check out of bounds condition on SetAttribute
TEST_F(InputStateTest, SetAttributeOutOfBounds) {
    // Control case, setting last attribute
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(kMaxVertexAttributes - 1, 0, dawn::VertexFormat::FloatR32, 0)
        .GetResult();

    // Test OOB
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(kMaxVertexAttributes, 0, dawn::VertexFormat::FloatR32, 0)
        .GetResult();
}

// Check that all attributes must be backed by an input
TEST_F(InputStateTest, RequireInputForAttribute) {
    // Control case
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 0)
        .GetResult();

    // Attribute 0 uses input 1 which doesn't exist
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 1, dawn::VertexFormat::FloatR32, 0)
        .GetResult();
}

// Check OOB checks for an attribute's input
TEST_F(InputStateTest, SetAttributeOOBCheckForInputs) {
    // Control case
    AssertWillBeSuccess(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32, 0)
        .GetResult();

    // Could crash if we didn't check for OOB
    AssertWillBeError(device.CreateInputStateBuilder())
        .SetInput(0, 0, dawn::InputStepMode::Vertex)
        .SetAttribute(0, 1000000, dawn::VertexFormat::FloatR32, 0)
        .GetResult();
}
