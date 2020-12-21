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

#include "dawn_native/SpirvUtils.h"

namespace dawn_native {

    spv::ExecutionModel ShaderStageToExecutionModel(SingleShaderStage stage) {
        switch (stage) {
            case SingleShaderStage::Vertex:
                return spv::ExecutionModelVertex;
            case SingleShaderStage::Fragment:
                return spv::ExecutionModelFragment;
            case SingleShaderStage::Compute:
                return spv::ExecutionModelGLCompute;
        }
    }

    SingleShaderStage ExecutionModelToShaderStage(spv::ExecutionModel model) {
        switch (model) {
            case spv::ExecutionModelVertex:
                return SingleShaderStage::Vertex;
            case spv::ExecutionModelFragment:
                return SingleShaderStage::Fragment;
            case spv::ExecutionModelGLCompute:
                return SingleShaderStage::Compute;
            default:
                UNREACHABLE();
        }
    }

    wgpu::TextureViewDimension SpirvDimToTextureViewDimension(spv::Dim dim, bool arrayed) {
        switch (dim) {
            case spv::Dim::Dim1D:
                return wgpu::TextureViewDimension::e1D;
            case spv::Dim::Dim2D:
                if (arrayed) {
                    return wgpu::TextureViewDimension::e2DArray;
                } else {
                    return wgpu::TextureViewDimension::e2D;
                }
            case spv::Dim::Dim3D:
                return wgpu::TextureViewDimension::e3D;
            case spv::Dim::DimCube:
                if (arrayed) {
                    return wgpu::TextureViewDimension::CubeArray;
                } else {
                    return wgpu::TextureViewDimension::Cube;
                }
            default:
                UNREACHABLE();
        }
    }

    wgpu::TextureFormat SpirvImageFormatToTextureFormat(spv::ImageFormat format) {
        switch (format) {
            case spv::ImageFormatR8:
                return wgpu::TextureFormat::R8Unorm;
            case spv::ImageFormatR8Snorm:
                return wgpu::TextureFormat::R8Snorm;
            case spv::ImageFormatR8ui:
                return wgpu::TextureFormat::R8Uint;
            case spv::ImageFormatR8i:
                return wgpu::TextureFormat::R8Sint;
            case spv::ImageFormatR16ui:
                return wgpu::TextureFormat::R16Uint;
            case spv::ImageFormatR16i:
                return wgpu::TextureFormat::R16Sint;
            case spv::ImageFormatR16f:
                return wgpu::TextureFormat::R16Float;
            case spv::ImageFormatRg8:
                return wgpu::TextureFormat::RG8Unorm;
            case spv::ImageFormatRg8Snorm:
                return wgpu::TextureFormat::RG8Snorm;
            case spv::ImageFormatRg8ui:
                return wgpu::TextureFormat::RG8Uint;
            case spv::ImageFormatRg8i:
                return wgpu::TextureFormat::RG8Sint;
            case spv::ImageFormatR32f:
                return wgpu::TextureFormat::R32Float;
            case spv::ImageFormatR32ui:
                return wgpu::TextureFormat::R32Uint;
            case spv::ImageFormatR32i:
                return wgpu::TextureFormat::R32Sint;
            case spv::ImageFormatRg16ui:
                return wgpu::TextureFormat::RG16Uint;
            case spv::ImageFormatRg16i:
                return wgpu::TextureFormat::RG16Sint;
            case spv::ImageFormatRg16f:
                return wgpu::TextureFormat::RG16Float;
            case spv::ImageFormatRgba8:
                return wgpu::TextureFormat::RGBA8Unorm;
            case spv::ImageFormatRgba8Snorm:
                return wgpu::TextureFormat::RGBA8Snorm;
            case spv::ImageFormatRgba8ui:
                return wgpu::TextureFormat::RGBA8Uint;
            case spv::ImageFormatRgba8i:
                return wgpu::TextureFormat::RGBA8Sint;
            case spv::ImageFormatRgb10A2:
                return wgpu::TextureFormat::RGB10A2Unorm;
            case spv::ImageFormatR11fG11fB10f:
                return wgpu::TextureFormat::RG11B10Ufloat;
            case spv::ImageFormatRg32f:
                return wgpu::TextureFormat::RG32Float;
            case spv::ImageFormatRg32ui:
                return wgpu::TextureFormat::RG32Uint;
            case spv::ImageFormatRg32i:
                return wgpu::TextureFormat::RG32Sint;
            case spv::ImageFormatRgba16ui:
                return wgpu::TextureFormat::RGBA16Uint;
            case spv::ImageFormatRgba16i:
                return wgpu::TextureFormat::RGBA16Sint;
            case spv::ImageFormatRgba16f:
                return wgpu::TextureFormat::RGBA16Float;
            case spv::ImageFormatRgba32f:
                return wgpu::TextureFormat::RGBA32Float;
            case spv::ImageFormatRgba32ui:
                return wgpu::TextureFormat::RGBA32Uint;
            case spv::ImageFormatRgba32i:
                return wgpu::TextureFormat::RGBA32Sint;
            default:
                return wgpu::TextureFormat::Undefined;
        }
    }

    wgpu::TextureComponentType SpirvBaseTypeToTextureComponentType(
        spirv_cross::SPIRType::BaseType spirvBaseType) {
        switch (spirvBaseType) {
            case spirv_cross::SPIRType::Float:
                return wgpu::TextureComponentType::Float;
            case spirv_cross::SPIRType::Int:
                return wgpu::TextureComponentType::Sint;
            case spirv_cross::SPIRType::UInt:
                return wgpu::TextureComponentType::Uint;
            default:
                UNREACHABLE();
        }
    }

    wgpu::TextureSampleType SpirvBaseTypeToTextureSampleType(
        spirv_cross::SPIRType::BaseType spirvBaseType) {
        switch (spirvBaseType) {
            case spirv_cross::SPIRType::Float:
                return wgpu::TextureSampleType::Float;
            case spirv_cross::SPIRType::Int:
                return wgpu::TextureSampleType::Sint;
            case spirv_cross::SPIRType::UInt:
                return wgpu::TextureSampleType::Uint;
            default:
                UNREACHABLE();
        }
    }

}  // namespace dawn_native
