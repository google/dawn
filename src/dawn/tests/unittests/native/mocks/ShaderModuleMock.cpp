// Copyright 2021 The Dawn Authors
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

#include "dawn/tests/unittests/native/mocks/ShaderModuleMock.h"

#include "dawn/native/ChainUtils.h"

namespace dawn::native {

using ::testing::NiceMock;

ShaderModuleMock::ShaderModuleMock(DeviceMock* device, const ShaderModuleDescriptor* descriptor)
    : ShaderModuleBase(device, descriptor) {
    ON_CALL(*this, DestroyImpl).WillByDefault([this]() { this->ShaderModuleBase::DestroyImpl(); });

    SetContentHash(ComputeContentHash());
}

ShaderModuleMock::~ShaderModuleMock() = default;

// static
Ref<ShaderModuleMock> ShaderModuleMock::Create(DeviceMock* device,
                                               const ShaderModuleDescriptor* descriptor) {
    Ref<ShaderModuleMock> shaderModule =
        AcquireRef(new NiceMock<ShaderModuleMock>(device, descriptor));
    ShaderModuleParseResult parseResult;
    ValidateAndParseShaderModule(device, descriptor, &parseResult, nullptr).AcquireSuccess();
    shaderModule->InitializeBase(&parseResult, nullptr).AcquireSuccess();
    return shaderModule;
}

// static
Ref<ShaderModuleMock> ShaderModuleMock::Create(DeviceMock* device, const char* source) {
    ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.code = source;
    ShaderModuleDescriptor desc = {};
    desc.nextInChain = &wgslDesc;
    return Create(device, &desc);
}

}  // namespace dawn::native
