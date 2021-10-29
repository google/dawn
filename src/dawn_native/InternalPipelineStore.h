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

#ifndef DAWNNATIVE_INTERNALPIPELINESTORE_H_
#define DAWNNATIVE_INTERNALPIPELINESTORE_H_

#include "dawn_native/ObjectBase.h"
#include "dawn_native/ScratchBuffer.h"
#include "dawn_native/dawn_platform.h"

#include <unordered_map>

namespace dawn_native {

    class DeviceBase;
    class RenderPipelineBase;
    class ShaderModuleBase;

    // Every DeviceBase owns an InternalPipelineStore. This is a general-purpose cache for
    // long-lived objects scoped to a device and used to support arbitrary pipeline operations.
    struct InternalPipelineStore {
        explicit InternalPipelineStore(DeviceBase* device);
        ~InternalPipelineStore();

        std::unordered_map<wgpu::TextureFormat, Ref<RenderPipelineBase>>
            copyTextureForBrowserPipelines;

        Ref<ShaderModuleBase> copyTextureForBrowser;

        Ref<ComputePipelineBase> timestampComputePipeline;
        Ref<ShaderModuleBase> timestampCS;

        Ref<ShaderModuleBase> dummyFragmentShader;

        // A scratch buffer suitable for use as a copy destination and storage binding.
        ScratchBuffer scratchStorage;

        // A scratch buffer suitable for use as a copy destination, storage binding, and indirect
        // buffer for indirect dispatch or draw calls.
        ScratchBuffer scratchIndirectStorage;

        Ref<ComputePipelineBase> renderValidationPipeline;
        Ref<ShaderModuleBase> renderValidationShader;
        Ref<ComputePipelineBase> dispatchIndirectValidationPipeline;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_INTERNALPIPELINESTORE_H_
