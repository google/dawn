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

#ifndef TESTS_UNITTESTS_NATIVE_MOCKS_BINDGROUP_MOCK_H_
#define TESTS_UNITTESTS_NATIVE_MOCKS_BINDGROUP_MOCK_H_

#include "dawn_native/BindGroup.h"
#include "dawn_native/Device.h"

#include <gmock/gmock.h>

namespace dawn_native {

    class BindGroupMock : public BindGroupBase {
      public:
        BindGroupMock(DeviceBase* device) : BindGroupBase(device) {
            ON_CALL(*this, DestroyImpl).WillByDefault([this]() {
                this->BindGroupBase::DestroyImpl();
            });
        }
        ~BindGroupMock() override = default;

        MOCK_METHOD(void, DestroyImpl, (), (override));
    };

}  // namespace dawn_native

#endif  // TESTS_UNITTESTS_NATIVE_MOCKS_BINDGROUP_MOCK_H_
