// Copyright 2018 The Dawn Authors
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

#include <sstream>
#include <string>

#include "dawn/common/Constants.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class ShaderModuleValidationTest : public ValidationTest {};

// Test case with a simpler shader that should successfully be created
TEST_F(ShaderModuleValidationTest, CreationSuccess) {
    const char* shader = R"(
                   OpCapability Shader
              %1 = OpExtInstImport "GLSL.std.450"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint Fragment %main "main" %fragColor
                   OpExecutionMode %main OriginUpperLeft
                   OpSource GLSL 450
                   OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
                   OpSourceExtension "GL_GOOGLE_include_directive"
                   OpName %main "main"
                   OpName %fragColor "fragColor"
                   OpDecorate %fragColor Location 0
           %void = OpTypeVoid
              %3 = OpTypeFunction %void
          %float = OpTypeFloat 32
        %v4float = OpTypeVector %float 4
    %_ptr_Output_v4float = OpTypePointer Output %v4float
      %fragColor = OpVariable %_ptr_Output_v4float Output
        %float_1 = OpConstant %float 1
        %float_0 = OpConstant %float 0
             %12 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
           %main = OpFunction %void None %3
              %5 = OpLabel
                   OpStore %fragColor %12
                   OpReturn
                   OpFunctionEnd)";

    utils::CreateShaderModuleFromASM(device, shader);
}

// Tests that if the output location exceeds kMaxColorAttachments the fragment shader will fail to
// be compiled.
TEST_F(ShaderModuleValidationTest, FragmentOutputLocationExceedsMaxColorAttachments) {
    std::ostringstream stream;
    stream << "@fragment fn main() -> @location(" << kMaxColorAttachments << R"() vec4<f32> {
            return vec4<f32>(0.0, 1.0, 0.0, 1.0);
        })";
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, stream.str().c_str()));
}

// Test that it is invalid to create a shader module with no chained descriptor. (It must be
// WGSL or SPIRV, not empty)
TEST_F(ShaderModuleValidationTest, NoChainedDescriptor) {
    wgpu::ShaderModuleDescriptor desc = {};
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&desc));
}

// Test that it is not allowed to use combined texture and sampler.
TEST_F(ShaderModuleValidationTest, CombinedTextureAndSampler) {
    // SPIR-V ASM produced by glslang for the following fragment shader:
    //
    //   #version 450
    //   layout(set = 0, binding = 0) uniform sampler2D tex;
    //   void main () {}
    //
    // Note that the following defines an interface combined texture/sampler which is not allowed
    // in Dawn / WebGPU.
    //
    //   %8 = OpTypeSampledImage %7
    //   %_ptr_UniformConstant_8 = OpTypePointer UniformConstant %8
    //   %tex = OpVariable %_ptr_UniformConstant_8 UniformConstant
    const char* shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %tex "tex"
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
%_ptr_UniformConstant_8 = OpTypePointer UniformConstant %8
        %tex = OpVariable %_ptr_UniformConstant_8 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    ASSERT_DEVICE_ERROR(utils::CreateShaderModuleFromASM(device, shader));
}

// Test that it is not allowed to declare a multisampled-array interface texture.
// TODO(enga): Also test multisampled cube, cube array, and 3D. These have no GLSL keywords.
TEST_F(ShaderModuleValidationTest, MultisampledArrayTexture) {
    // SPIR-V ASM produced by glslang for the following fragment shader:
    //
    //  #version 450
    //  layout(set=0, binding=0) uniform texture2DMSArray tex;
    //  void main () {}}
    //
    // Note that the following defines an interface array multisampled texture which is not allowed
    // in Dawn / WebGPU.
    //
    //  %7 = OpTypeImage %float 2D 0 1 1 1 Unknown
    //  %_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
    //  %tex = OpVariable %_ptr_UniformConstant_7 UniformConstant
    const char* shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %tex "tex"
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 1 1 1 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
        %tex = OpVariable %_ptr_UniformConstant_7 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    ASSERT_DEVICE_ERROR(utils::CreateShaderModuleFromASM(device, shader));
}

// Tests that shader module compilation messages can be queried.
TEST_F(ShaderModuleValidationTest, GetCompilationMessages) {
    // This test works assuming ShaderModule is backed by a dawn::native::ShaderModuleBase, which
    // is not the case on the wire.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(0.0, 1.0, 0.0, 1.0);
        })");

    dawn::native::ShaderModuleBase* shaderModuleBase = dawn::native::FromAPI(shaderModule.Get());
    dawn::native::OwnedCompilationMessages* messages = shaderModuleBase->GetCompilationMessages();
    messages->ClearMessages();
    messages->AddMessageForTesting("Info Message");
    messages->AddMessageForTesting("Warning Message", wgpu::CompilationMessageType::Warning);
    messages->AddMessageForTesting("Error Message", wgpu::CompilationMessageType::Error, 3, 4);
    messages->AddMessageForTesting("Complete Message", wgpu::CompilationMessageType::Info, 3, 4, 5,
                                   6);

    auto callback = [](WGPUCompilationInfoRequestStatus status, const WGPUCompilationInfo* info,
                       void* userdata) {
        ASSERT_EQ(WGPUCompilationInfoRequestStatus_Success, status);
        ASSERT_NE(nullptr, info);
        ASSERT_EQ(4u, info->messageCount);

        const WGPUCompilationMessage* message = &info->messages[0];
        ASSERT_STREQ("Info Message", message->message);
        ASSERT_EQ(WGPUCompilationMessageType_Info, message->type);
        ASSERT_EQ(0u, message->lineNum);
        ASSERT_EQ(0u, message->linePos);

        message = &info->messages[1];
        ASSERT_STREQ("Warning Message", message->message);
        ASSERT_EQ(WGPUCompilationMessageType_Warning, message->type);
        ASSERT_EQ(0u, message->lineNum);
        ASSERT_EQ(0u, message->linePos);

        message = &info->messages[2];
        ASSERT_STREQ("Error Message", message->message);
        ASSERT_EQ(WGPUCompilationMessageType_Error, message->type);
        ASSERT_EQ(3u, message->lineNum);
        ASSERT_EQ(4u, message->linePos);

        message = &info->messages[3];
        ASSERT_STREQ("Complete Message", message->message);
        ASSERT_EQ(WGPUCompilationMessageType_Info, message->type);
        ASSERT_EQ(3u, message->lineNum);
        ASSERT_EQ(4u, message->linePos);
        ASSERT_EQ(5u, message->offset);
        ASSERT_EQ(6u, message->length);
    };

    shaderModule.GetCompilationInfo(callback, nullptr);
}

// Validate the maximum location of effective inter-stage variables cannot be greater than 14
// (kMaxInterStageShaderComponents / 4 - 1).
TEST_F(ShaderModuleValidationTest, MaximumShaderIOLocations) {
    auto CheckTestPipeline = [&](bool success, uint32_t maximumOutputLocation,
                                 wgpu::ShaderStage failingShaderStage) {
        // Build the ShaderIO struct containing variables up to maximumOutputLocation.
        std::ostringstream stream;
        stream << "struct ShaderIO {" << std::endl;
        for (uint32_t location = 1; location <= maximumOutputLocation; ++location) {
            stream << "@location(" << location << ") var" << location << ": f32," << std::endl;
        }

        if (failingShaderStage == wgpu::ShaderStage::Vertex) {
            stream << " @builtin(position) pos: vec4<f32>,";
        }
        stream << "}\n";

        std::string ioStruct = stream.str();

        // Build the test pipeline. Note that it's not possible with just ASSERT_DEVICE_ERROR
        // whether it is the vertex or fragment shader that fails. So instead we will look for the
        // string "failingVertex" or "failingFragment" in the error message.
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        const char* errorMatcher = nullptr;
        switch (failingShaderStage) {
            case wgpu::ShaderStage::Vertex: {
                errorMatcher = "failingVertex";
                pDesc.vertex.entryPoint = "failingVertex";
                pDesc.vertex.module = utils::CreateShaderModule(device, (ioStruct + R"(
                    @vertex fn failingVertex() -> ShaderIO {
                        var shaderIO : ShaderIO;
                        shaderIO.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                        return shaderIO;
                     }
                )")
                                                                            .c_str());
                pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                    @fragment fn main() -> @location(0) vec4<f32> {
                        return vec4<f32>(0.0);
                    }
                )");
                break;
            }

            case wgpu::ShaderStage::Fragment: {
                errorMatcher = "failingFragment";
                pDesc.cFragment.entryPoint = "failingFragment";
                pDesc.cFragment.module = utils::CreateShaderModule(device, (ioStruct + R"(
                    @fragment fn failingFragment(io : ShaderIO) -> @location(0) vec4<f32> {
                        return vec4<f32>(0.0);
                     }
                )")
                                                                               .c_str());
                pDesc.vertex.module = utils::CreateShaderModule(device, R"(
                    @vertex fn main() -> @builtin(position) vec4<f32> {
                        return vec4<f32>(0.0);
                    }
                )");
                break;
            }

            default:
                UNREACHABLE();
        }

        if (success) {
            if (failingShaderStage == wgpu::ShaderStage::Vertex) {
                // It is allowed that fragment inputs are a subset of the vertex output variables.
                device.CreateRenderPipeline(&pDesc);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                    testing::HasSubstr("The fragment input at location"));
            }
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                testing::HasSubstr(errorMatcher));
        }
    };

    // It is allowed to create a shader module with the maximum active vertex output location ==
    // (kMaxInterStageShaderVariables - 1);
    CheckTestPipeline(true, kMaxInterStageShaderVariables - 1, wgpu::ShaderStage::Vertex);

    // It isn't allowed to create a shader module with the maximum active vertex output location ==
    // kMaxInterStageShaderVariables;
    CheckTestPipeline(false, kMaxInterStageShaderVariables, wgpu::ShaderStage::Vertex);

    // It is allowed to create a shader module with the maximum active fragment input location ==
    // (kMaxInterStageShaderVariables - 1);
    CheckTestPipeline(true, kMaxInterStageShaderVariables - 1, wgpu::ShaderStage::Fragment);

    // It isn't allowed to create a shader module with the maximum active vertex output location ==
    // kMaxInterStageShaderVariables;
    CheckTestPipeline(false, kMaxInterStageShaderVariables, wgpu::ShaderStage::Fragment);
}

// Validate the maximum number of total inter-stage user-defined variable component count and
// built-in variables cannot exceed kMaxInterStageShaderComponents.
TEST_F(ShaderModuleValidationTest, MaximumInterStageShaderComponents) {
    auto CheckTestPipeline = [&](bool success,
                                 uint32_t totalUserDefinedInterStageShaderComponentCount,
                                 wgpu::ShaderStage failingShaderStage,
                                 const char* extraBuiltInDeclarations = "",
                                 bool usePointListAsPrimitiveType = false) {
        // Build the ShaderIO struct containing totalUserDefinedInterStageShaderComponentCount
        // components. Components are added in two parts, a bunch of vec4s, then one additional
        // variable for the remaining components.
        std::ostringstream stream;
        stream << "struct ShaderIO {" << std::endl << extraBuiltInDeclarations << std::endl;
        uint32_t vec4InputLocations = totalUserDefinedInterStageShaderComponentCount / 4;

        for (uint32_t location = 0; location < vec4InputLocations; ++location) {
            stream << "@location(" << location << ") var" << location << ": vec4<f32>,"
                   << std::endl;
        }

        uint32_t lastComponentCount = totalUserDefinedInterStageShaderComponentCount % 4;
        if (lastComponentCount > 0) {
            stream << "@location(" << vec4InputLocations << ") var" << vec4InputLocations << ": ";
            if (lastComponentCount == 1) {
                stream << "f32,";
            } else {
                stream << " vec" << lastComponentCount << "<f32>,";
            }
            stream << std::endl;
        }

        if (failingShaderStage == wgpu::ShaderStage::Vertex) {
            stream << " @builtin(position) pos: vec4<f32>,";
        }
        stream << "}\n";

        std::string ioStruct = stream.str();

        // Build the test pipeline. Note that it's not possible with just ASSERT_DEVICE_ERROR
        // whether it is the vertex or fragment shader that fails. So instead we will look for the
        // string "failingVertex" or "failingFragment" in the error message.
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        if (usePointListAsPrimitiveType) {
            pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        } else {
            pDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        }

        const char* errorMatcher = nullptr;
        switch (failingShaderStage) {
            case wgpu::ShaderStage::Vertex: {
                if (usePointListAsPrimitiveType) {
                    errorMatcher = "PointList";
                } else {
                    errorMatcher = "failingVertex";
                }
                pDesc.vertex.entryPoint = "failingVertex";
                pDesc.vertex.module = utils::CreateShaderModule(device, (ioStruct + R"(
                    @vertex fn failingVertex() -> ShaderIO {
                        var shaderIO : ShaderIO;
                        shaderIO.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                        return shaderIO;
                     }
                )")
                                                                            .c_str());
                pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                    @fragment fn main() -> @location(0) vec4<f32> {
                        return vec4<f32>(0.0);
                    }
                )");
                break;
            }

            case wgpu::ShaderStage::Fragment: {
                errorMatcher = "failingFragment";
                pDesc.cFragment.entryPoint = "failingFragment";
                pDesc.cFragment.module = utils::CreateShaderModule(device, (ioStruct + R"(
                    @fragment fn failingFragment(io : ShaderIO) -> @location(0) vec4<f32> {
                        return vec4<f32>(0.0);
                     }
                )")
                                                                               .c_str());
                pDesc.vertex.module = utils::CreateShaderModule(device, R"(
                    @vertex fn main() -> @builtin(position) vec4<f32> {
                        return vec4<f32>(0.0);
                    }
                )");
                break;
            }

            default:
                UNREACHABLE();
        }

        if (success) {
            if (failingShaderStage == wgpu::ShaderStage::Vertex) {
                // It is allowed that fragment inputs are a subset of the vertex output variables.
                device.CreateRenderPipeline(&pDesc);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                    testing::HasSubstr("The fragment input at location"));
            }
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                testing::HasSubstr(errorMatcher));
        }
    };

    // Verify when there is no input builtin variable in a fragment shader, the total user-defined
    // input component count must be less than kMaxInterStageShaderComponents.
    {
        CheckTestPipeline(true, kMaxInterStageShaderComponents, wgpu::ShaderStage::Fragment);
        CheckTestPipeline(false, kMaxInterStageShaderComponents + 1, wgpu::ShaderStage::Fragment);
    }

    // Verify the total user-defined vertex output component count must be less than
    // kMaxInterStageShaderComponents.
    {
        CheckTestPipeline(true, kMaxInterStageShaderComponents, wgpu::ShaderStage::Vertex);
        CheckTestPipeline(false, kMaxInterStageShaderComponents + 1, wgpu::ShaderStage::Vertex);
    }

    // Verify the total user-defined vertex output component count must be less than
    // (kMaxInterStageShaderComponents - 1) when the primitive topology is PointList.
    {
        constexpr bool kUsePointListAsPrimitiveTopology = true;
        const char* kExtraBuiltins = "";
        CheckTestPipeline(true, kMaxInterStageShaderComponents - 1, wgpu::ShaderStage::Vertex,
                          kExtraBuiltins, kUsePointListAsPrimitiveTopology);
        CheckTestPipeline(false, kMaxInterStageShaderComponents, wgpu::ShaderStage::Vertex,
                          kExtraBuiltins, kUsePointListAsPrimitiveTopology);
    }

    // @builtin(position) in fragment shaders shouldn't be counted into the maximum inter-stage
    // component count.
    {
        CheckTestPipeline(true, kMaxInterStageShaderComponents, wgpu::ShaderStage::Fragment,
                          "@builtin(position) fragCoord : vec4<f32>,");
    }

    // @builtin(front_facing) should be counted into the maximum inter-stage component count.
    {
        CheckTestPipeline(true, kMaxInterStageShaderComponents - 1, wgpu::ShaderStage::Fragment,
                          "@builtin(front_facing) frontFacing : bool,");
        CheckTestPipeline(false, kMaxInterStageShaderComponents, wgpu::ShaderStage::Fragment,
                          "@builtin(front_facing) frontFacing : bool,");
    }

    // @builtin(sample_index) should be counted into the maximum inter-stage component count.
    {
        CheckTestPipeline(true, kMaxInterStageShaderComponents - 1, wgpu::ShaderStage::Fragment,
                          "@builtin(sample_index) sampleIndex : u32,");
        CheckTestPipeline(false, kMaxInterStageShaderComponents, wgpu::ShaderStage::Fragment,
                          "@builtin(sample_index) sampleIndex : u32,");
    }

    // @builtin(sample_mask) should be counted into the maximum inter-stage component count.
    {
        CheckTestPipeline(true, kMaxInterStageShaderComponents - 1, wgpu::ShaderStage::Fragment,
                          "@builtin(sample_mask) sampleMask : u32,");
        CheckTestPipeline(false, kMaxInterStageShaderComponents, wgpu::ShaderStage::Fragment,
                          "@builtin(sample_mask) sampleMask : u32,");
    }
}

// Test that numeric ID must be unique
TEST_F(ShaderModuleValidationTest, OverridableConstantsNumericIDConflicts) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
@id(1234) override c0: u32;
@id(1234) override c1: u32;

struct Buf {
    data : array<u32, 2>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    // make sure the overridable constants are not optimized out
    buf.data[0] = c0;
    buf.data[1] = c1;
})"));
}

// Test that @binding must be less then kMaxBindingsPerBindGroup
TEST_F(ShaderModuleValidationTest, MaxBindingNumber) {
    static_assert(kMaxBindingsPerBindGroup == 640);

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.entryPoint = "main";

    // kMaxBindingsPerBindGroup-1 is valid.
    desc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(639) var s : sampler;
        @compute @workgroup_size(1) fn main() {
            _ = s;
        }
    )");
    device.CreateComputePipeline(&desc);

    // kMaxBindingsPerBindGroup is an error
    desc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(640) var s : sampler;
        @compute @workgroup_size(1) fn main() {
            _ = s;
        }
    )");
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&desc));
}

// Test that missing decorations on shader IO or bindings causes a validation error.
TEST_F(ShaderModuleValidationTest, MissingDecorations) {
    // Vertex input.
    utils::CreateShaderModule(device, R"(
        @vertex fn main(@location(0) a : vec4<f32>) -> @builtin(position) vec4<f32> {
            return vec4(1.0);
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @vertex fn main(a : vec4<f32>) -> @builtin(position) vec4<f32> {
            return vec4(1.0);
        }
    )"));

    // Vertex output
    utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(position) pos : vec4<f32>,
            @location(0) a : f32,
        }
        @vertex fn main() -> Output {
            var output : Output;
            return output;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(position) pos : vec4<f32>,
            a : f32,
        }
        @vertex fn main() -> Output {
            var output : Output;
            return output;
        }
    )"));

    // Fragment input
    utils::CreateShaderModule(device, R"(
        @fragment fn main(@location(0) a : vec4<f32>) -> @location(0) f32 {
            return 1.0;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @fragment fn main(a : vec4<f32>) -> @location(0) f32 {
            return 1.0;
        }
    )"));

    // Fragment input
    utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) f32 {
            return 1.0;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @fragment fn main() -> f32 {
            return 1.0;
        }
    )"));

    // Binding decorations
    utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var s : sampler;
        @fragment fn main() -> @location(0) f32 {
            _ = s;
            return 1.0;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @binding(0) var s : sampler;
        @fragment fn main() -> @location(0) f32 {
            _ = s;
            return 1.0;
        }
    )"));
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @group(0) var s : sampler;
        @fragment fn main() -> @location(0) f32 {
            _ = s;
            return 1.0;
        }
    )"));
}

// Test that WGSL extension used by enable directives must be allowed by WebGPU.
TEST_F(ShaderModuleValidationTest, ExtensionMustBeAllowed) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
enable f16;

@compute @workgroup_size(1) fn main() {})"));
}
