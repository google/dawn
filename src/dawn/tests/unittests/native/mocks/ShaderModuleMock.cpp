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

namespace dawn::native {

ShaderModuleMock::ShaderModuleMock(DeviceBase* device) : ShaderModuleBase(device) {
    ON_CALL(*this, DestroyImpl).WillByDefault([this]() { this->ShaderModuleBase::DestroyImpl(); });
}

ResultOrError<Ref<ShaderModuleMock>> ShaderModuleMock::Create(DeviceBase* device,
                                                              const char* source) {
    ShaderModuleMock* mock = new ShaderModuleMock(device);

    ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.source = source;
    ShaderModuleDescriptor desc;
    desc.nextInChain = &wgslDesc;

    ShaderModuleParseResult parseResult;
    DAWN_TRY(ValidateShaderModuleDescriptor(device, &desc, &parseResult, nullptr));
    DAWN_TRY(mock->InitializeBase(&parseResult));
    return AcquireRef(mock);
}

}  // namespace dawn::native
