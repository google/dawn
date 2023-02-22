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

#ifndef SRC_DAWN_NATIVE_COMPUTEPIPELINE_H_
#define SRC_DAWN_NATIVE_COMPUTEPIPELINE_H_

#include "dawn/common/NonCopyable.h"
#include "dawn/native/Forward.h"
#include "dawn/native/Pipeline.h"

namespace dawn::native {

class DeviceBase;
struct EntryPointMetadata;

MaybeError ValidateComputePipelineDescriptor(DeviceBase* device,
                                             const ComputePipelineDescriptor* descriptor);

class ComputePipelineBase : public PipelineBase {
  public:
    ComputePipelineBase(DeviceBase* device, const ComputePipelineDescriptor* descriptor);
    ~ComputePipelineBase() override;

    static ComputePipelineBase* MakeError(DeviceBase* device);

    ObjectType GetType() const override;

    // Functors necessary for the unordered_set<ComputePipelineBase*>-based cache.
    struct EqualityFunc {
        bool operator()(const ComputePipelineBase* a, const ComputePipelineBase* b) const;
    };

  protected:
    void DestroyImpl() override;

  private:
    ComputePipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMPUTEPIPELINE_H_
