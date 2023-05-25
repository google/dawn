// Copyright 2023 The Dawn Authors
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

#include <limits>
#include <string>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CompatValidationTest : public ValidationTest {
  protected:
    bool UseCompatibilityMode() const override { return true; }
};

TEST_F(CompatValidationTest, CanNotCreateCubeArrayTextureView) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 6};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture cubeTexture = device.CreateTexture(&descriptor);

    {
        wgpu::TextureViewDescriptor cubeViewDescriptor;
        cubeViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
        cubeViewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;

        cubeTexture.CreateView(&cubeViewDescriptor);
    }

    {
        wgpu::TextureViewDescriptor cubeArrayViewDescriptor;
        cubeArrayViewDescriptor.dimension = wgpu::TextureViewDimension::CubeArray;
        cubeArrayViewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;

        ASSERT_DEVICE_ERROR(cubeTexture.CreateView(&cubeArrayViewDescriptor));
    }

    cubeTexture.Destroy();
}

TEST_F(CompatValidationTest, CanNotCreatePipelineWithDifferentPerTargetBlendStateOrWriteMask) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0);
        }

        struct FragmentOut {
            @location(0) fragColor0 : vec4f,
            @location(1) fragColor1 : vec4f,
            @location(2) fragColor2 : vec4f,
        }

        @fragment fn fs() -> FragmentOut {
            var output : FragmentOut;
            output.fragColor0 = vec4f(0);
            output.fragColor1 = vec4f(0);
            output.fragColor2 = vec4f(0);
            return output;
        }
    )");

    utils::ComboRenderPipelineDescriptor testDescriptor;
    testDescriptor.layout = {};
    testDescriptor.vertex.module = module;
    testDescriptor.vertex.entryPoint = "vs";
    testDescriptor.cFragment.module = module;
    testDescriptor.cFragment.entryPoint = "fs";
    testDescriptor.cFragment.targetCount = 3;
    testDescriptor.cTargets[1].format = wgpu::TextureFormat::Undefined;

    for (int i = 0; i < 10; ++i) {
        wgpu::BlendState blend0;
        wgpu::BlendState blend2;

        // Blend state intentionally omitted for target 1
        testDescriptor.cTargets[0].blend = &blend0;
        testDescriptor.cTargets[2].blend = &blend2;

        bool expectError = true;
        switch (i) {
            case 0:  // default
                expectError = false;
                break;
            case 1:  // no blend
                testDescriptor.cTargets[0].blend = nullptr;
                break;
            case 2:  // no blend second target
                testDescriptor.cTargets[2].blend = nullptr;
                break;
            case 3:  // color.operation
                blend2.color.operation = wgpu::BlendOperation::Subtract;
                break;
            case 4:  // color.srcFactor
                blend2.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
                break;
            case 5:  // color.dstFactor
                blend2.color.dstFactor = wgpu::BlendFactor::DstAlpha;
                break;
            case 6:  // alpha.operation
                blend2.alpha.operation = wgpu::BlendOperation::Subtract;
                break;
            case 7:  // alpha.srcFactor
                blend2.alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
                break;
            case 8:  // alpha.dstFactor
                blend2.alpha.dstFactor = wgpu::BlendFactor::DstAlpha;
                break;
            case 9:  // writeMask
                testDescriptor.cTargets[2].writeMask = wgpu::ColorWriteMask::Green;
                break;
            default:
                UNREACHABLE();
        }

        if (expectError) {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&testDescriptor));
        } else {
            device.CreateRenderPipeline(&testDescriptor);
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
