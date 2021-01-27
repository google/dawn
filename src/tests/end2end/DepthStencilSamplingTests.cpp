// Copyright 2020 The Dawn Authors
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

#include "common/Assert.h"
#include "tests/DawnTest.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

namespace {

    constexpr wgpu::TextureFormat kDepthFormats[] = {
        wgpu::TextureFormat::Depth32Float,
        wgpu::TextureFormat::Depth24Plus,
        wgpu::TextureFormat::Depth24PlusStencil8,
    };

    constexpr wgpu::TextureFormat kStencilFormats[] = {
        wgpu::TextureFormat::Depth24PlusStencil8,
    };

    constexpr wgpu::CompareFunction kCompareFunctions[] = {
        wgpu::CompareFunction::Never,        wgpu::CompareFunction::Less,
        wgpu::CompareFunction::LessEqual,    wgpu::CompareFunction::Greater,
        wgpu::CompareFunction::GreaterEqual, wgpu::CompareFunction::Equal,
        wgpu::CompareFunction::NotEqual,     wgpu::CompareFunction::Always,
    };

    // Test a "normal" ref value between 0 and 1; as well as negative and > 1 refs.
    constexpr float kCompareRefs[] = {-0.1, 0.4, 1.2};

    // Test 0, below the ref, equal to, above the ref, and 1.
    const std::vector<float> kNormalizedTextureValues = {0.0, 0.3, 0.4, 0.5, 1.0};

    // Test the limits, and some values in between.
    const std::vector<uint8_t> kStencilValues = {uint8_t(0), uint8_t(1), uint8_t(38), uint8_t(255)};

}  // anonymous namespace

class DepthStencilSamplingTest : public DawnTest {
  protected:
    enum class TestAspect {
        Depth,
        Stencil,
    };

    void SetUp() override {
        DawnTest::SetUp();

        wgpu::BufferDescriptor uniformBufferDesc;
        uniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        uniformBufferDesc.size = sizeof(float);
        mUniformBuffer = device.CreateBuffer(&uniformBufferDesc);
    }

    wgpu::RenderPipeline CreateSamplingRenderPipeline(std::vector<TestAspect> aspects,
                                                      uint32_t componentIndex) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        std::ostringstream shaderSource;
        std::ostringstream shaderBody;

        uint32_t index = 0;
        for (TestAspect aspect : aspects) {
            switch (aspect) {
                case TestAspect::Depth:
                    shaderSource << "[[group(0), binding(" << index
                                 << ")]] var<uniform_constant> tex" << index
                                 << " : texture_2d<f32>;\n";

                    shaderSource << "[[location(" << index << ")]] var<out> result" << index
                                 << " : f32;\n";

                    shaderBody << "\nresult" << index << " = textureLoad(tex" << index
                               << ", vec2<i32>(0, 0), 0)[" << componentIndex << "];\n";
                    pipelineDescriptor.cColorStates[index].format = wgpu::TextureFormat::R32Float;
                    break;
                case TestAspect::Stencil:
                    shaderSource << "[[group(0), binding(" << index
                                 << ")]] var<uniform_constant> tex" << index
                                 << " : texture_2d<u32>;\n";

                    shaderSource << "[[location(" << index << ")]] var<out> result" << index
                                 << " : u32;\n";

                    shaderBody << "\nresult" << index << " = textureLoad(tex" << index
                               << ", vec2<i32>(0, 0), 0)[" << componentIndex << "];\n";
                    pipelineDescriptor.cColorStates[index].format = wgpu::TextureFormat::R8Uint;
                    break;
            }

            index++;
        }

        shaderSource << "[[stage(fragment)]] fn main() -> void { " << shaderBody.str() << "\n}";

        wgpu::ShaderModule fsModule =
            utils::CreateShaderModuleFromWGSL(device, shaderSource.str().c_str());
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        pipelineDescriptor.colorStateCount = static_cast<uint32_t>(aspects.size());

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateSamplingComputePipeline(std::vector<TestAspect> aspects,
                                                        uint32_t componentIndex) {
        std::ostringstream shaderSource;
        std::ostringstream shaderBody;
        shaderSource << R"(
            [[block]] struct DepthResult {
                [[offset(0)]] value : f32;
            };
            [[block]] struct StencilResult {
                [[offset(0)]] value : u32;
            };)";
        shaderSource << "\n";

        uint32_t index = 0;
        for (TestAspect aspect : aspects) {
            switch (aspect) {
                case TestAspect::Depth:
                    shaderSource << "[[group(0), binding(" << 2 * index
                                 << ")]] var<uniform_constant> tex" << index
                                 << " : texture_2d<f32>;\n";

                    shaderSource << "[[group(0), binding(" << 2 * index + 1
                                 << ")]] var<storage_buffer> result" << index
                                 << " : DepthResult;\n";

                    shaderBody << "\nresult" << index << ".value = textureLoad(tex" << index
                               << ", vec2<i32>(0, 0), 0)[" << componentIndex << "];";
                    break;
                case TestAspect::Stencil:
                    shaderSource << "[[group(0), binding(" << 2 * index
                                 << ")]] var<uniform_constant> tex" << index
                                 << " : texture_2d<u32>;\n";

                    shaderSource << "[[group(0), binding(" << 2 * index + 1
                                 << ")]] var<storage_buffer> result" << index
                                 << " : StencilResult;\n";

                    shaderBody << "\nresult" << index << ".value = textureLoad(tex" << index
                               << ", vec2<i32>(0, 0), 0)[" << componentIndex << "];";
                    break;
            }

            index++;
        }

        shaderSource << "[[stage(compute)]] fn main() -> void { " << shaderBody.str() << "\n}";

        wgpu::ShaderModule csModule =
            utils::CreateShaderModuleFromWGSL(device, shaderSource.str().c_str());

        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.computeStage.module = csModule;
        pipelineDescriptor.computeStage.entryPoint = "main";

        return device.CreateComputePipeline(&pipelineDescriptor);
    }

    wgpu::RenderPipeline CreateComparisonRenderPipeline() {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[group(0), binding(0)]] var<uniform_constant> samp : sampler_comparison;
            [[group(0), binding(1)]] var<uniform_constant> tex : texture_depth_2d;
            [[block]] struct Uniforms {
                [[offset(0)]] compareRef : f32;
            };
            [[group(0), binding(2)]] var<uniform> uniforms : Uniforms;

            [[location(0)]] var<out> samplerResult : f32;

            [[stage(fragment)]] fn main() -> void {
                samplerResult = textureSampleCompare(tex, samp, vec2<f32>(0.5, 0.5), uniforms.compareRef);
            })");

        // TODO(dawn:367): Cannot use GetBindGroupLayout for comparison samplers without shader
        // reflection data.
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Comparison},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth},
                     {2, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        pipelineDescriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        pipelineDescriptor.cColorStates[0].format = wgpu::TextureFormat::R32Float;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComparisonComputePipeline() {
        wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[group(0), binding(0)]] var<uniform_constant> samp : sampler_comparison;
            [[group(0), binding(1)]] var<uniform_constant> tex : texture_depth_2d;
            [[block]] struct Uniforms {
                [[offset(0)]] compareRef : f32;
            };
            [[group(0), binding(2)]] var<uniform> uniforms : Uniforms;

            [[block]] struct SamplerResult {
                [[offset(0)]] value : f32;
            };
            [[group(0), binding(3)]] var<storage_buffer> samplerResult : SamplerResult;

            [[stage(compute)]] fn main() -> void {
                samplerResult.value = textureSampleCompare(tex, samp, vec2<f32>(0.5, 0.5), uniforms.compareRef);
            })");

        // TODO(dawn:367): Cannot use GetBindGroupLayout without shader reflection data.
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::SamplerBindingType::Comparison},
                     {1, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Depth},
                     {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                     {3, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        pipelineDescriptor.computeStage.module = csModule;
        pipelineDescriptor.computeStage.entryPoint = "main";

        return device.CreateComputePipeline(&pipelineDescriptor);
    }

    wgpu::Texture CreateInputTexture(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor inputTextureDesc;
        inputTextureDesc.usage = wgpu::TextureUsage::Sampled | wgpu::TextureUsage::OutputAttachment;
        inputTextureDesc.size = {1, 1, 1};
        inputTextureDesc.format = format;
        return device.CreateTexture(&inputTextureDesc);
    }

    wgpu::Texture CreateOutputTexture(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor outputTextureDesc;
        outputTextureDesc.usage =
            wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        outputTextureDesc.size = {1, 1, 1};
        outputTextureDesc.format = format;
        return device.CreateTexture(&outputTextureDesc);
    }

    wgpu::Buffer CreateOutputBuffer() {
        wgpu::BufferDescriptor outputBufferDesc;
        outputBufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        outputBufferDesc.size = sizeof(float);  // Large enough for both float and uint8_t
        return device.CreateBuffer(&outputBufferDesc);
    }

    void UpdateInputDepth(wgpu::CommandEncoder commandEncoder,
                          wgpu::Texture texture,
                          float depthValue) {
        utils::ComboRenderPassDescriptor passDescriptor({}, texture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.clearDepth = depthValue;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.EndPass();
    }

    void UpdateInputStencil(wgpu::CommandEncoder commandEncoder,
                            wgpu::Texture texture,
                            uint8_t stencilValue) {
        utils::ComboRenderPassDescriptor passDescriptor({}, texture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.clearStencil = stencilValue;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.EndPass();
    }

    template <typename T>
    void DoSamplingTest(TestAspect aspect,
                        wgpu::RenderPipeline pipeline,
                        wgpu::TextureFormat format,
                        std::vector<T> textureValues,
                        std::vector<T> expectedValues) {
        ASSERT(textureValues.size() == expectedValues.size());

        wgpu::Texture outputTexture;
        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        switch (aspect) {
            case TestAspect::Depth:
                inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
                outputTexture = CreateOutputTexture(wgpu::TextureFormat::R32Float);
                break;
            case TestAspect::Stencil:
                inputViewDesc.aspect = wgpu::TextureAspect::StencilOnly;
                outputTexture = CreateOutputTexture(wgpu::TextureFormat::R8Uint);
                break;
        }

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, inputTexture.CreateView(&inputViewDesc)}});

        for (size_t i = 0; i < textureValues.size(); ++i) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            switch (aspect) {
                case TestAspect::Depth:
                    UpdateInputDepth(commandEncoder, inputTexture, textureValues[i]);
                    break;
                case TestAspect::Stencil:
                    UpdateInputStencil(commandEncoder, inputTexture, textureValues[i]);
                    break;
            }

            // Render into the output texture
            {
                utils::ComboRenderPassDescriptor passDescriptor({outputTexture.CreateView()});
                wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(1);
                pass.EndPass();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_TEXTURE_EQ(expectedValues[i], outputTexture, 0, 0);
        }
    }

    template <typename T>
    void DoSamplingTest(TestAspect aspect,
                        wgpu::ComputePipeline pipeline,
                        wgpu::TextureFormat format,
                        std::vector<T> textureValues,
                        std::vector<T> expectedValues) {
        ASSERT(textureValues.size() == expectedValues.size());

        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        switch (aspect) {
            case TestAspect::Depth:
                inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
                break;
            case TestAspect::Stencil:
                inputViewDesc.aspect = wgpu::TextureAspect::StencilOnly;
                break;
        }

        wgpu::Buffer outputBuffer = CreateOutputBuffer();

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, inputTexture.CreateView(&inputViewDesc)}, {1, outputBuffer}});

        for (size_t i = 0; i < textureValues.size(); ++i) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            switch (aspect) {
                case TestAspect::Depth:
                    UpdateInputDepth(commandEncoder, inputTexture, textureValues[i]);
                    break;
                case TestAspect::Stencil:
                    UpdateInputStencil(commandEncoder, inputTexture, textureValues[i]);
                    break;
            }

            // Sample into the output buffer
            {
                wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.Dispatch(1);
                pass.EndPass();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            uint32_t expectedValueU32 = 0;
            memcpy(&expectedValueU32, &expectedValues[i], std::min(sizeof(T), sizeof(uint32_t)));
            EXPECT_BUFFER_U32_EQ(expectedValueU32, outputBuffer, 0);
        }
    }

    template <typename T>
    void DoSamplingTest(TestAspect aspect,
                        wgpu::RenderPipeline pipeline,
                        wgpu::TextureFormat format,
                        std::vector<T> textureValues) {
        DoSamplingTest(aspect, pipeline, format, textureValues, textureValues);
    }

    template <typename T>
    void DoSamplingTest(TestAspect aspect,
                        wgpu::ComputePipeline pipeline,
                        wgpu::TextureFormat format,
                        std::vector<T> textureValues) {
        DoSamplingTest(aspect, pipeline, format, textureValues, textureValues);
    }

    static bool CompareFunctionPasses(float compareRef,
                                      wgpu::CompareFunction compare,
                                      float textureValue) {
        switch (compare) {
            case wgpu::CompareFunction::Never:
                return false;
            case wgpu::CompareFunction::Less:
                return compareRef < textureValue;
            case wgpu::CompareFunction::LessEqual:
                return compareRef <= textureValue;
            case wgpu::CompareFunction::Greater:
                return compareRef > textureValue;
            case wgpu::CompareFunction::GreaterEqual:
                return compareRef >= textureValue;
            case wgpu::CompareFunction::Equal:
                return compareRef == textureValue;
            case wgpu::CompareFunction::NotEqual:
                return compareRef != textureValue;
            case wgpu::CompareFunction::Always:
                return true;
            default:
                return false;
        }
    }

    void DoDepthCompareRefTest(wgpu::RenderPipeline pipeline,
                               wgpu::TextureFormat format,
                               float compareRef,
                               wgpu::CompareFunction compare,
                               std::vector<float> textureValues) {
        queue.WriteBuffer(mUniformBuffer, 0, &compareRef, sizeof(float));

        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.compare = compare;
        wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, sampler},
                                     {1, inputTexture.CreateView(&inputViewDesc)},
                                     {2, mUniformBuffer},
                                 });

        wgpu::Texture outputTexture = CreateOutputTexture(wgpu::TextureFormat::R32Float);
        for (float textureValue : textureValues) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            UpdateInputDepth(commandEncoder, inputTexture, textureValue);

            // Render into the output texture
            {
                utils::ComboRenderPassDescriptor passDescriptor({outputTexture.CreateView()});
                wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(1);
                pass.EndPass();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_TEXTURE_EQ(CompareFunctionPasses(compareRef, compare, textureValue) ? 1.f : 0.f,
                              outputTexture, 0, 0);
        }
    }

    void DoDepthCompareRefTest(wgpu::ComputePipeline pipeline,
                               wgpu::TextureFormat format,
                               float compareRef,
                               wgpu::CompareFunction compare,
                               std::vector<float> textureValues) {
        queue.WriteBuffer(mUniformBuffer, 0, &compareRef, sizeof(float));

        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.compare = compare;
        wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::Buffer outputBuffer = CreateOutputBuffer();

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, sampler},
                                  {1, inputTexture.CreateView(&inputViewDesc)},
                                  {2, mUniformBuffer},
                                  {3, outputBuffer}});

        for (float textureValue : textureValues) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            UpdateInputDepth(commandEncoder, inputTexture, textureValue);

            // Sample into the output buffer
            {
                wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.Dispatch(1);
                pass.EndPass();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            float float0 = 0.f;
            float float1 = 1.f;
            float* expected =
                CompareFunctionPasses(compareRef, compare, textureValue) ? &float1 : &float0;

            EXPECT_BUFFER_U32_EQ(*reinterpret_cast<uint32_t*>(expected), outputBuffer, 0);
        }
    }

  private:
    wgpu::Buffer mUniformBuffer;
};

// Test that sampling a depth texture with a render/compute pipeline works
TEST_P(DepthStencilSamplingTest, SampleDepth) {
    for (wgpu::TextureFormat format : kDepthFormats) {
        // Test 0, between [0, 1], and 1.
        DoSamplingTest(TestAspect::Depth, CreateSamplingRenderPipeline({TestAspect::Depth}, 0),
                       format, kNormalizedTextureValues);

        DoSamplingTest(TestAspect::Depth, CreateSamplingComputePipeline({TestAspect::Depth}, 0),
                       format, kNormalizedTextureValues);
    }
}

// Test that sampling a stencil texture with a render/compute pipeline works
TEST_P(DepthStencilSamplingTest, SampleStencil) {
    // TODO(crbug.com/dawn/593): This test requires glTextureView, which is unsupported on GLES.
    DAWN_SKIP_TEST_IF(IsOpenGLES());
    for (wgpu::TextureFormat format : kStencilFormats) {
        DoSamplingTest(TestAspect::Stencil, CreateSamplingRenderPipeline({TestAspect::Stencil}, 0),
                       format, kStencilValues);

        DoSamplingTest(TestAspect::Stencil, CreateSamplingComputePipeline({TestAspect::Stencil}, 0),
                       format, kStencilValues);
    }
}

// Test that sampling a depth/stencil texture at components 1, 2, and 3 yield 0, 0, and 1
// respectively
TEST_P(DepthStencilSamplingTest, SampleExtraComponents) {
    // TODO(crbug.com/dawn/593): This test requires glTextureView, which is unsupported on GLES.
    DAWN_SKIP_TEST_IF(IsOpenGLES());
    // TODO(enga): In Metal, color textures' unspecified default components values
    // are (0, 0, 0, 1). Depth/stencil textures are undefined! Figure out what
    // to do here.
    // See Section 6.10 of the Metal Shading Language Specification
    DAWN_SKIP_TEST_IF(IsMetal());

    float expectedDepth[4] = {0, 0, 0, 1};
    uint8_t expectedStencil[4] = {0, 0, 0, 1};

    for (uint32_t component : {1, 2, 3}) {
        DoSamplingTest<float>(
            TestAspect::Depth, CreateSamplingRenderPipeline({TestAspect::Depth}, component),
            wgpu::TextureFormat::Depth24PlusStencil8, {0.2f}, {expectedDepth[component]});

        DoSamplingTest<float>(
            TestAspect::Depth, CreateSamplingComputePipeline({TestAspect::Depth}, component),
            wgpu::TextureFormat::Depth24PlusStencil8, {0.2f}, {expectedDepth[component]});

        DoSamplingTest<uint8_t>(
            TestAspect::Stencil, CreateSamplingRenderPipeline({TestAspect::Stencil}, component),
            wgpu::TextureFormat::Depth24PlusStencil8, {uint8_t(37)}, {expectedStencil[component]});

        DoSamplingTest<uint8_t>(
            TestAspect::Stencil, CreateSamplingComputePipeline({TestAspect::Stencil}, component),
            wgpu::TextureFormat::Depth24PlusStencil8, {uint8_t(37)}, {expectedStencil[component]});
    }
}

// Test sampling both depth and stencil with a render/compute pipeline works.
TEST_P(DepthStencilSamplingTest, SampleDepthAndStencilRender) {
    // TODO(crbug.com/dawn/593): This test requires glTextureView, which is unsupported on GLES.
    DAWN_SKIP_TEST_IF(IsOpenGLES());
    wgpu::SamplerDescriptor samplerDesc;
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::Texture inputTexture = CreateInputTexture(wgpu::TextureFormat::Depth24PlusStencil8);

    wgpu::TextureViewDescriptor depthViewDesc = {};
    depthViewDesc.aspect = wgpu::TextureAspect::DepthOnly;

    wgpu::TextureViewDescriptor stencilViewDesc = {};
    stencilViewDesc.aspect = wgpu::TextureAspect::StencilOnly;

    // With render pipeline
    {
        wgpu::RenderPipeline pipeline =
            CreateSamplingRenderPipeline({TestAspect::Depth, TestAspect::Stencil}, 0);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, inputTexture.CreateView(&depthViewDesc)},
                                     {1, inputTexture.CreateView(&stencilViewDesc)},
                                 });

        wgpu::Texture depthOutput = CreateOutputTexture(wgpu::TextureFormat::R32Float);
        wgpu::Texture stencilOutput = CreateOutputTexture(wgpu::TextureFormat::R8Uint);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Initialize both depth and stencil aspects.
        utils::ComboRenderPassDescriptor passDescriptor({}, inputTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.clearDepth = 0.43f;
        passDescriptor.cDepthStencilAttachmentInfo.clearStencil = 31;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.EndPass();

        // Render into the output textures
        {
            utils::ComboRenderPassDescriptor passDescriptor(
                {depthOutput.CreateView(), stencilOutput.CreateView()});
            wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(1);
            pass.EndPass();
        }

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_TEXTURE_EQ(passDescriptor.cDepthStencilAttachmentInfo.clearDepth, depthOutput, 0, 0);
        EXPECT_TEXTURE_EQ(uint8_t(passDescriptor.cDepthStencilAttachmentInfo.clearStencil),
                          stencilOutput, 0, 0);
    }

    // With compute pipeline
    {
        wgpu::ComputePipeline pipeline =
            CreateSamplingComputePipeline({TestAspect::Depth, TestAspect::Stencil}, 0);

        wgpu::Buffer depthOutput = CreateOutputBuffer();
        wgpu::Buffer stencilOutput = CreateOutputBuffer();

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, inputTexture.CreateView(&depthViewDesc)},
                                  {1, depthOutput},
                                  {2, inputTexture.CreateView(&stencilViewDesc)},
                                  {3, stencilOutput}});

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        // Initialize both depth and stencil aspects.
        utils::ComboRenderPassDescriptor passDescriptor({}, inputTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.clearDepth = 0.43f;
        passDescriptor.cDepthStencilAttachmentInfo.clearStencil = 31;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.EndPass();

        // Sample into the output buffers
        {
            wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Dispatch(1);
            pass.EndPass();
        }

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        uint32_t expectedValueU32 = 0;
        memcpy(&expectedValueU32, &passDescriptor.cDepthStencilAttachmentInfo.clearDepth,
               sizeof(float));
        EXPECT_BUFFER_U32_EQ(expectedValueU32, depthOutput, 0);

        expectedValueU32 = 0;
        memcpy(&expectedValueU32, &passDescriptor.cDepthStencilAttachmentInfo.clearStencil,
               sizeof(uint8_t));
        EXPECT_BUFFER_U32_EQ(expectedValueU32, stencilOutput, 0);
    }
}

// Test that sampling in a render pipeline with all of the compare functions works.
TEST_P(DepthStencilSamplingTest, CompareFunctionsRender) {
    // Initialization via renderPass loadOp doesn't work on Mac Intel.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    wgpu::RenderPipeline pipeline = CreateComparisonRenderPipeline();

    for (wgpu::TextureFormat format : kDepthFormats) {
        // Test a "normal" ref value between 0 and 1; as well as negative and > 1 refs.
        for (float compareRef : kCompareRefs) {
            // Test 0, below the ref, equal to, above the ref, and 1.
            for (wgpu::CompareFunction f : kCompareFunctions) {
                DoDepthCompareRefTest(pipeline, format, compareRef, f, kNormalizedTextureValues);
            }
        }
    }
}

// Test that sampling in a render pipeline with all of the compare functions works.
TEST_P(DepthStencilSamplingTest, CompareFunctionsCompute) {
    // Initialization via renderPass loadOp doesn't work on Mac Intel.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    wgpu::ComputePipeline pipeline = CreateComparisonComputePipeline();

    for (wgpu::TextureFormat format : kDepthFormats) {
        // Test a "normal" ref value between 0 and 1; as well as negative and > 1 refs.
        for (float compareRef : kCompareRefs) {
            // Test 0, below the ref, equal to, above the ref, and 1.
            for (wgpu::CompareFunction f : kCompareFunctions) {
                DoDepthCompareRefTest(pipeline, format, compareRef, f, kNormalizedTextureValues);
            }
        }
    }
}

DAWN_INSTANTIATE_TEST(DepthStencilSamplingTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
