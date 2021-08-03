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

#include "dawn_native/opengl/ComputePipelineGL.h"

#include "dawn_native/opengl/DeviceGL.h"

namespace dawn_native { namespace opengl {

    // static
    ResultOrError<Ref<ComputePipeline>> ComputePipeline::Create(
        Device* device,
        const ComputePipelineDescriptor* descriptor) {
        Ref<ComputePipeline> pipeline = AcquireRef(new ComputePipeline(device, descriptor));
        DAWN_TRY(pipeline->Initialize(descriptor));
        return pipeline;
    }

    MaybeError ComputePipeline::Initialize(const ComputePipelineDescriptor*) {
        DAWN_TRY(
            InitializeBase(ToBackend(GetDevice())->gl, ToBackend(GetLayout()), GetAllStages()));
        return {};
    }

    void ComputePipeline::ApplyNow() {
        PipelineGL::ApplyNow(ToBackend(GetDevice())->gl);
    }

}}  // namespace dawn_native::opengl
