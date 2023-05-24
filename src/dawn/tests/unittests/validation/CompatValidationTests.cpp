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

}  // anonymous namespace
}  // namespace dawn
