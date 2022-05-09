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

#ifndef SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_EXTERNALTEXTUREMOCK_H_
#define SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_EXTERNALTEXTUREMOCK_H_

#include "gmock/gmock.h"

#include "dawn/native/Device.h"
#include "dawn/native/ExternalTexture.h"

namespace dawn::native {

class ExternalTextureMock : public ExternalTextureBase {
  public:
    explicit ExternalTextureMock(DeviceBase* device);
    ~ExternalTextureMock() override;

    MOCK_METHOD(void, DestroyImpl, (), (override));
};

}  // namespace dawn::native

#endif  // SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_EXTERNALTEXTUREMOCK_H_
