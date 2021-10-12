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

#include <gtest/gtest.h>

#include "dawn_native/Toggles.h"
#include "mocks/BindGroupLayoutMock.h"
#include "mocks/DeviceMock.h"
#include "tests/DawnNativeTest.h"

namespace dawn_native { namespace {

    using ::testing::ByMove;
    using ::testing::InSequence;
    using ::testing::Return;

    TEST(DestroyObjectTests, BindGroupLayout) {
        // Skipping validation on descriptors as coverage for validation is already present.
        DeviceMock device;
        device.SetToggle(Toggle::SkipValidation, true);

        BindGroupLayoutMock* bindGroupLayoutMock = new BindGroupLayoutMock(&device);
        EXPECT_CALL(*bindGroupLayoutMock, DestroyApiObjectImpl).Times(1);

        BindGroupLayoutDescriptor desc = {};
        Ref<BindGroupLayoutBase> bindGroupLayout;
        EXPECT_CALL(device, CreateBindGroupLayoutImpl)
            .WillOnce(Return(ByMove(AcquireRef(bindGroupLayoutMock))));
        DAWN_ASSERT_AND_ASSIGN(bindGroupLayout, device.CreateBindGroupLayout(&desc));

        EXPECT_TRUE(bindGroupLayout->IsAlive());
        EXPECT_TRUE(bindGroupLayout->IsCachedReference());

        bindGroupLayout->DestroyApiObject();
        EXPECT_FALSE(bindGroupLayout->IsAlive());
    }

    // Destroying the objects on the device should result in all created objects being destroyed in
    // order.
    TEST(DestroyObjectTests, DestroyObjects) {
        DeviceMock device;
        device.SetToggle(Toggle::SkipValidation, true);

        BindGroupLayoutMock* bindGroupLayoutMock = new BindGroupLayoutMock(&device);
        {
            InSequence seq;
            EXPECT_CALL(*bindGroupLayoutMock, DestroyApiObjectImpl).Times(1);
        }

        BindGroupLayoutDescriptor desc = {};
        Ref<BindGroupLayoutBase> bindGroupLayout;
        EXPECT_CALL(device, CreateBindGroupLayoutImpl)
            .WillOnce(Return(ByMove(AcquireRef(bindGroupLayoutMock))));
        DAWN_ASSERT_AND_ASSIGN(bindGroupLayout, device.CreateBindGroupLayout(&desc));
        EXPECT_TRUE(bindGroupLayout->IsAlive());
        EXPECT_TRUE(bindGroupLayout->IsCachedReference());

        device.DestroyObjects();
        EXPECT_FALSE(bindGroupLayout->IsAlive());
    }

}}  // namespace dawn_native::
