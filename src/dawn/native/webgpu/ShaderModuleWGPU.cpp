// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/webgpu/ShaderModuleWGPU.h"

#include <utility>

#include "dawn/native/ShaderModule.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    DAWN_TRY(module->Initialize(descriptor, parseResult));
    return module;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)),
      ObjectWGPU(device->wgpu.shaderModuleRelease) {}

MaybeError ShaderModule::Initialize(const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                    ShaderModuleParseResult* parseResult) {
    DAWN_TRY(InitializeBase(parseResult));

    if (GetCompilationMessages()->HasWarningsOrErrors()) {
        // Cached parse result already shows it is an invalid shader.
        // No need to create the real shader module on the backend.
        return {};
    }

    auto desc = ToAPI(*descriptor);
    mInnerHandle =
        ToBackend(GetDevice())
            ->wgpu.deviceCreateShaderModule(ToBackend(GetDevice())->GetInnerHandle(), desc);
    DAWN_ASSERT(mInnerHandle);

    return {};
}

}  // namespace dawn::native::webgpu
