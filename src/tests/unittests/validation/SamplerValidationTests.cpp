// Copyright 2019 The Dawn Authors
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

#include "utils/WGPUHelpers.h"

#include <cmath>

namespace {

    class SamplerValidationTest : public ValidationTest {};

    // Test NaN and INFINITY values are not allowed
    TEST_F(SamplerValidationTest, InvalidLOD) {
        {
            wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
            device.CreateSampler(&samplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
            samplerDesc.lodMinClamp = NAN;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
        {
            wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
            samplerDesc.lodMaxClamp = NAN;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
        {
            wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
            samplerDesc.lodMaxClamp = INFINITY;
            device.CreateSampler(&samplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
            samplerDesc.lodMaxClamp = INFINITY;
            samplerDesc.lodMinClamp = INFINITY;
            device.CreateSampler(&samplerDesc);
        }
    }

    TEST_F(SamplerValidationTest, InvalidFilterAnisotropic) {
        wgpu::SamplerDescriptor kValidAnisoSamplerDesc = {};
        kValidAnisoSamplerDesc.maxAnisotropy = 2;
        kValidAnisoSamplerDesc.minFilter = wgpu::FilterMode::Linear;
        kValidAnisoSamplerDesc.magFilter = wgpu::FilterMode::Linear;
        kValidAnisoSamplerDesc.mipmapFilter = wgpu::FilterMode::Linear;
        {
            // when maxAnisotropy > 1, min, mag, mipmap filter should be linear
            device.CreateSampler(&kValidAnisoSamplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.maxAnisotropy = 0;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.minFilter = wgpu::FilterMode::Nearest;
            samplerDesc.magFilter = wgpu::FilterMode::Nearest;
            samplerDesc.mipmapFilter = wgpu::FilterMode::Nearest;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.minFilter = wgpu::FilterMode::Nearest;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.magFilter = wgpu::FilterMode::Nearest;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.mipmapFilter = wgpu::FilterMode::Nearest;
            ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
        }
    }

    TEST_F(SamplerValidationTest, ValidFilterAnisotropic) {
        wgpu::SamplerDescriptor kValidAnisoSamplerDesc = {};
        kValidAnisoSamplerDesc.maxAnisotropy = 2;
        kValidAnisoSamplerDesc.minFilter = wgpu::FilterMode::Linear;
        kValidAnisoSamplerDesc.magFilter = wgpu::FilterMode::Linear;
        kValidAnisoSamplerDesc.mipmapFilter = wgpu::FilterMode::Linear;
        { device.CreateSampler(); }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.maxAnisotropy = 16;
            device.CreateSampler(&samplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.maxAnisotropy = 32;
            device.CreateSampler(&samplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.maxAnisotropy = 0x7FFF;
            device.CreateSampler(&samplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.maxAnisotropy = 0x8000;
            device.CreateSampler(&samplerDesc);
        }
        {
            wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
            samplerDesc.maxAnisotropy = 0xFFFF;
            device.CreateSampler(&samplerDesc);
        }
    }

}  // anonymous namespace
