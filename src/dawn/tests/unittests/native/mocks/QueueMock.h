// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_QUEUEMOCK_H_
#define SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_QUEUEMOCK_H_

#include "gmock/gmock.h"

#include "dawn/native/Queue.h"

namespace dawn::native {

class DeviceMock;

class QueueMock : public QueueBase {
  public:
    QueueMock(DeviceMock* device, const QueueDescriptor* descriptor);
    ~QueueMock() override;

    MOCK_METHOD(MaybeError, SubmitImpl, (uint32_t, CommandBufferBase* const*), (override));
    MOCK_METHOD(MaybeError,
                WriteBufferImpl,
                (BufferBase*, uint64_t, const void*, size_t),
                (override));
    MOCK_METHOD(MaybeError,
                WriteTextureImpl,
                (const ImageCopyTexture&, const void*, const TextureDataLayout&, const Extent3D&),
                (override));
    MOCK_METHOD(void, DestroyImpl, (), (override));

    MOCK_METHOD(ResultOrError<ExecutionSerial>, CheckAndUpdateCompletedSerials, (), (override));
    MOCK_METHOD(bool, HasPendingCommands, (), (const, override));
    MOCK_METHOD(void, ForceEventualFlushOfCommands, (), (override));
    MOCK_METHOD(MaybeError, WaitForIdleForDestruction, (), (override));
};

}  // namespace dawn::native

#endif  // SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_QUEUEMOCK_H_
