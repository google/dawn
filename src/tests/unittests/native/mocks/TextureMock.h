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

#ifndef TESTS_UNITTESTS_NATIVE_MOCKS_TEXTURE_MOCK_H_
#define TESTS_UNITTESTS_NATIVE_MOCKS_TEXTURE_MOCK_H_

#include "dawn_native/Device.h"
#include "dawn_native/Texture.h"

#include <gmock/gmock.h>

namespace dawn_native {

    class TextureMock : public TextureBase {
      public:
        TextureMock(DeviceBase* device, TextureBase::TextureState state)
            : TextureBase(device, state) {
            ON_CALL(*this, DestroyImpl).WillByDefault([this]() {
                this->TextureBase::DestroyImpl();
            });
        }
        ~TextureMock() override = default;

        MOCK_METHOD(void, DestroyImpl, (), (override));
    };

    class TextureViewMock : public TextureViewBase {
      public:
        TextureViewMock(TextureBase* texture) : TextureViewBase(texture) {
        }
        ~TextureViewMock() override = default;

        MOCK_METHOD(void, DestroyImpl, (), (override));
    };

}  // namespace dawn_native

#endif  // TESTS_UNITTESTS_NATIVE_MOCKS_TEXTURE_MOCK_H_
