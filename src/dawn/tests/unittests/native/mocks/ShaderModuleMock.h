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

#ifndef SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_SHADERMODULEMOCK_H_
#define SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_SHADERMODULEMOCK_H_

#include <memory>

#include "gmock/gmock.h"

#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

class ShaderModuleMock : public ShaderModuleBase {
  public:
    explicit ShaderModuleMock(DeviceBase* device);
    ~ShaderModuleMock() override;

    MOCK_METHOD(void, DestroyImpl, (), (override));

    // Creates a shader module mock based on the wgsl source.
    static ResultOrError<Ref<ShaderModuleMock>> Create(DeviceBase* device, const char* source);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_SHADERMODULEMOCK_H_
