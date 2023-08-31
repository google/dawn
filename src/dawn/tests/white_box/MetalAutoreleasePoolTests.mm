// Copyright 2020 The Dawn Authors
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

#include "dawn/tests/DawnTest.h"

#include "dawn/native/Device.h"
#include "dawn/native/metal/Forward.h"
#include "dawn/native/metal/QueueMTL.h"

namespace dawn::native {
namespace {

using namespace metal;

class MetalAutoreleasePoolTests : public DawnTest {
  private:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        mMtlQueue = ToBackend(FromAPI(device.Get())->GetQueue());
    }

  protected:
    Queue* mMtlQueue = nullptr;
};

// Test that the MTLCommandBuffer owned by the pending command context can
// outlive an autoreleasepool block.
TEST_P(MetalAutoreleasePoolTests, CommandBufferOutlivesAutorelease) {
    @autoreleasepool {
        // Get the recording context which will allocate a MTLCommandBuffer.
        // It will get autoreleased at the end of this block.
        mMtlQueue->GetPendingCommandContext();
    }

    // Submitting the command buffer should succeed.
    ASSERT_TRUE(mMtlQueue->SubmitPendingCommandBuffer().IsSuccess());
}

// Test that the MTLBlitCommandEncoder owned by the pending command context
// can outlive an autoreleasepool block.
TEST_P(MetalAutoreleasePoolTests, EncoderOutlivesAutorelease) {
    @autoreleasepool {
        // Get the recording context which will allocate a MTLCommandBuffer.
        // Begin a blit encoder.
        // Both will get autoreleased at the end of this block.
        mMtlQueue->GetPendingCommandContext()->EnsureBlit();
    }

    // Submitting the command buffer should succeed.
    mMtlQueue->GetPendingCommandContext()->EndBlit();
    ASSERT_TRUE(mMtlQueue->SubmitPendingCommandBuffer().IsSuccess());
}

DAWN_INSTANTIATE_TEST(MetalAutoreleasePoolTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn::native
