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

#include "dawn/native/opengl/ComputePipelineGL.h"

#include "dawn/native/opengl/DeviceGL.h"

namespace dawn::native::opengl {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const ComputePipelineDescriptor* descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::DestroyImpl() {
    ComputePipelineBase::DestroyImpl();
    DeleteProgram(ToBackend(GetDevice())->GetGL());
}

MaybeError ComputePipeline::Initialize() {
    DAWN_TRY(
        InitializeBase(ToBackend(GetDevice())->GetGL(), ToBackend(GetLayout()), GetAllStages()));
    return {};
}

void ComputePipeline::ApplyNow() {
    PipelineGL::ApplyNow(ToBackend(GetDevice())->GetGL());
}

}  // namespace dawn::native::opengl
