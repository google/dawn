// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/Ref.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mocks/BufferMock.h"
#include "mocks/DawnMockTest.h"

using testing::_;
using testing::ByMove;
using testing::Eq;
using testing::Return;

namespace dawn::native {
namespace {

using MockMapAsyncCallback =
    testing::StrictMock<testing::MockCppCallback<void (*)(wgpu::MapAsyncStatus, wgpu::StringView)>>;

class BufferBaseTest : public DawnMockTest {};

// Test that MapAsyncImpl() throwing an error causes map async callback to return an error status.
TEST_F(BufferBaseTest, MapAsyncImplError) {
    constexpr std::string_view kErrorText = "Platform error";

    BufferDescriptor desc = {};
    desc.size = 16;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    Ref<BufferMock> bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
    EXPECT_CALL(*bufferMock.Get(), MapAsyncImpl).WillOnce([&]() -> MaybeError {
        return DAWN_FORMAT_INTERNAL_ERROR(kErrorText);
    });

    // Internal error will cause device loss as well.
    EXPECT_CALL(mDeviceLostCallback,
                Call(_, wgpu::DeviceLostReason::Unknown, testing::HasSubstr(kErrorText)))
        .Times(1);

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(Eq(wgpu::MapAsyncStatus::Error), Eq(kErrorText)));
    {
        EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));
        wgpu::Buffer buffer = device.CreateBuffer(ToCppAPI(&desc));
        buffer.MapAsync(wgpu::MapMode::Write, 0, desc.size, wgpu::CallbackMode::AllowSpontaneous,
                        cb.Callback());
    }
}

}  // namespace
}  // namespace dawn::native
