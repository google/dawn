// Copyright 2022 The Dawn Authors
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

#include "dawn/tests/unittests/native/mocks/CommandBufferMock.h"

#include "dawn/native/CommandEncoder.h"

namespace dawn::native {

CommandBufferMock::CommandBufferMock(DeviceMock* device,
                                     CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {
    // Make sure that the command encoder was also created using the mock device since it is not
    // directly passed in.
    ASSERT(device == encoder->GetDevice());

    ON_CALL(*this, DestroyImpl).WillByDefault([this]() { this->CommandBufferBase::DestroyImpl(); });
}

CommandBufferMock::~CommandBufferMock() = default;

}  // namespace dawn::native
