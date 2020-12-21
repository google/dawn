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

// This file contains utilities to convert from-to spirv.hpp datatypes without polluting other
// headers with spirv.hpp

#ifndef DAWNNATIVE_SPIRV_UTILS_H_
#define DAWNNATIVE_SPIRV_UTILS_H_

#include "dawn_native/Format.h"
#include "dawn_native/PerStage.h"
#include "dawn_native/dawn_platform.h"

#include <spirv_cross.hpp>

namespace dawn_native {

    // Returns the spirv_cross equivalent for this shader stage and vice-versa.
    spv::ExecutionModel ShaderStageToExecutionModel(SingleShaderStage stage);
    SingleShaderStage ExecutionModelToShaderStage(spv::ExecutionModel model);

    // Returns the texture view dimension for corresponding to (dim, arrayed).
    wgpu::TextureViewDimension SpirvDimToTextureViewDimension(spv::Dim dim, bool arrayed);

    // Returns the texture format corresponding to format.
    wgpu::TextureFormat SpirvImageFormatToTextureFormat(spv::ImageFormat format);

    // Returns the format "component type" corresponding to the SPIRV base type.
    wgpu::TextureComponentType SpirvBaseTypeToTextureComponentType(
        spirv_cross::SPIRType::BaseType spirvBaseType);
    wgpu::TextureSampleType SpirvBaseTypeToTextureSampleType(
        spirv_cross::SPIRType::BaseType spirvBaseType);

}  // namespace dawn_native

#endif  // DAWNNATIVE_SPIRV_UTILS_H_
