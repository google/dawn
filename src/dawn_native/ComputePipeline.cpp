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

#include "dawn_native/ComputePipeline.h"

#include "dawn_native/Device.h"

namespace dawn_native {

    MaybeError ValidateComputePipelineDescriptor(DeviceBase*,
                                                 const ComputePipelineDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        if (descriptor->module == nullptr) {
            return DAWN_VALIDATION_ERROR("module cannot be null");
        }

        if (descriptor->layout == nullptr) {
            return DAWN_VALIDATION_ERROR("layout cannot be null");
        }

        if (descriptor->entryPoint != std::string("main")) {
            return DAWN_VALIDATION_ERROR("Currently the entry point has to be main()");
        }

        if (descriptor->module->GetExecutionModel() != dawn::ShaderStage::Compute) {
            return DAWN_VALIDATION_ERROR("Setting module with wrong execution model");
        }

        if (!descriptor->module->IsCompatibleWithPipelineLayout(descriptor->layout)) {
            return DAWN_VALIDATION_ERROR("Stage not compatible with layout");
        }

        return {};
    }

    // ComputePipelineBase

    ComputePipelineBase::ComputePipelineBase(DeviceBase* device,
                                             const ComputePipelineDescriptor* descriptor)
        : PipelineBase(device, descriptor->layout, dawn::ShaderStageBit::Compute) {
        ExtractModuleData(dawn::ShaderStage::Compute, descriptor->module);
    }

}  // namespace dawn_native
