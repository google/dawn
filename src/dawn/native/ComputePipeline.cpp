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

#include "dawn/native/ComputePipeline.h"

#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"

namespace dawn::native {

MaybeError ValidateComputePipelineDescriptor(DeviceBase* device,
                                             const ComputePipelineDescriptor* descriptor) {
    if (descriptor->nextInChain != nullptr) {
        return DAWN_VALIDATION_ERROR("nextInChain must be nullptr.");
    }

    if (descriptor->layout != nullptr) {
        DAWN_TRY(device->ValidateObject(descriptor->layout));
    }

    DAWN_TRY_CONTEXT(ValidateProgrammableStage(
                         device, descriptor->compute.module, descriptor->compute.entryPoint,
                         descriptor->compute.constantCount, descriptor->compute.constants,
                         descriptor->layout, SingleShaderStage::Compute),
                     "validating compute stage (%s, entryPoint: %s).", descriptor->compute.module,
                     descriptor->compute.entryPoint);
    return {};
}

// ComputePipelineBase

ComputePipelineBase::ComputePipelineBase(DeviceBase* device,
                                         const ComputePipelineDescriptor* descriptor)
    : PipelineBase(
          device,
          descriptor->layout,
          descriptor->label,
          {{SingleShaderStage::Compute, descriptor->compute.module, descriptor->compute.entryPoint,
            descriptor->compute.constantCount, descriptor->compute.constants}}) {
    SetContentHash(ComputeContentHash());
    GetObjectTrackingList()->Track(this);

    // Initialize the cache key to include the cache type and device information.
    StreamIn(&mCacheKey, CacheKey::Type::ComputePipeline, device->GetCacheKey());
}

ComputePipelineBase::ComputePipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : PipelineBase(device, tag) {}

ComputePipelineBase::~ComputePipelineBase() = default;

void ComputePipelineBase::DestroyImpl() {
    if (IsCachedReference()) {
        // Do not uncache the actual cached object if we are a blueprint.
        GetDevice()->UncacheComputePipeline(this);
    }
}

// static
ComputePipelineBase* ComputePipelineBase::MakeError(DeviceBase* device) {
    class ErrorComputePipeline final : public ComputePipelineBase {
      public:
        explicit ErrorComputePipeline(DeviceBase* device)
            : ComputePipelineBase(device, ObjectBase::kError) {}

        MaybeError Initialize() override {
            UNREACHABLE();
            return {};
        }
    };

    return new ErrorComputePipeline(device);
}

ObjectType ComputePipelineBase::GetType() const {
    return ObjectType::ComputePipeline;
}

bool ComputePipelineBase::EqualityFunc::operator()(const ComputePipelineBase* a,
                                                   const ComputePipelineBase* b) const {
    return PipelineBase::EqualForCache(a, b);
}

}  // namespace dawn::native
