// Copyright 2022 The Dawn & Tint Authors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <utility>

#include "dawn/native/DawnNative.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/tests/MockCallback.h"
#include "src/dawn/tests/StringViewMatchers.h"
#include "src/dawn/tests/unittests/native/mocks/ComputePipelineMock.h"
#include "src/dawn/tests/unittests/native/mocks/DawnMockTest.h"
#include "src/dawn/tests/unittests/native/mocks/RenderPipelineMock.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn::native {
namespace {

using ::testing::_;
using ::testing::ByMove;
using ::testing::Combine;
using ::testing::EmptySizedString;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::IsNull;
using ::testing::Mock;
using ::testing::MockCppCallback;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Values;
using ::testing::WithParamInterface;

using MockComputePipelineAsyncCallback = MockCppCallback<
    void (*)(wgpu::CreatePipelineAsyncStatus, wgpu::ComputePipeline, wgpu::StringView)>;
using MockRenderPipelineAsyncCallback = MockCppCallback<
    void (*)(wgpu::CreatePipelineAsyncStatus, wgpu::RenderPipeline, wgpu::StringView)>;

static constexpr std::string_view kComputeShader = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kVertexShader = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )";

static constexpr std::string_view kErrorMessage = "Initialization error";

class CreatePipelineAsyncEventTests : public DawnMockTest {
  protected:
    MockComputePipelineAsyncCallback mockComputePipelineCb;
    MockRenderPipelineAsyncCallback mockRenderPipelineCb;
};

class CreatePipelineAsyncAfterDeviceLossTests
    : public CreatePipelineAsyncEventTests,
      public WithParamInterface<std::tuple<wgpu::DeviceLostReason, wgpu::CallbackMode>> {
  protected:
    void SetUp() override {
        CreatePipelineAsyncEventTests::SetUp();
        wgpu::DeviceLostReason reason = std::get<0>(GetParam());
        EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), reason, _)).Times(1);
        if (reason == wgpu::DeviceLostReason::Destroyed) {
            device.Destroy();
        } else {
            device.ForceLoss(reason, "Device lost for testing");
        }
        ASSERT_TRUE(mDeviceMock->IsLost());
    }

    wgpu::CallbackMode GetCallbackMode() const { return std::get<1>(GetParam()); }

    void CompleteFuture(wgpu::Future future) {
        // Only spontaneous callbacks may have fired during pipeline creation.
        EXPECT_EQ(mCallbackCalled, GetCallbackMode() == wgpu::CallbackMode::AllowSpontaneous);
        switch (GetCallbackMode()) {
            case wgpu::CallbackMode::WaitAnyOnly: {
                ProcessEvents();
                EXPECT_FALSE(mCallbackCalled);
                wgpu::Instance instance(ToAPI(mDeviceMock->GetInstance()));
                EXPECT_EQ(instance.WaitAny(future, 0), wgpu::WaitStatus::Success);
                break;
            }
            case wgpu::CallbackMode::AllowProcessEvents:
                ProcessEvents();
                break;
            case wgpu::CallbackMode::AllowSpontaneous:
                // A ready-at-creation event must already have called the callback.
                break;
        }
        EXPECT_TRUE(mCallbackCalled);
    }

    bool mCallbackCalled = false;
};

INSTANTIATE_TEST_SUITE_P(,
                         CreatePipelineAsyncAfterDeviceLossTests,
                         Combine(Values(wgpu::DeviceLostReason::Unknown,
                                        wgpu::DeviceLostReason::Destroyed),
                                 Values(wgpu::CallbackMode::WaitAnyOnly,
                                        wgpu::CallbackMode::AllowProcessEvents,
                                        wgpu::CallbackMode::AllowSpontaneous)));

// Even an invalid descriptor must resolve successfully after device loss, without backend work.
TEST_P(CreatePipelineAsyncAfterDeviceLossTests, CreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.label = "lost compute pipeline";
    // No shader module: this descriptor would fail validation on a live device.
    EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl).Times(0);
    EXPECT_CALL(mockComputePipelineCb,
                Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce([this](wgpu::CreatePipelineAsyncStatus, wgpu::ComputePipeline pipeline,
                         wgpu::StringView) {
            mCallbackCalled = true;
            ASSERT_NE(pipeline, nullptr);
            EXPECT_TRUE(CheckIsErrorForTesting(pipeline.Get()));
            EXPECT_EQ(FromAPI(pipeline.Get())->GetLabel(), "lost compute pipeline");
        });

    wgpu::Future future = device.CreateComputePipelineAsync(&desc, GetCallbackMode(),
                                                            mockComputePipelineCb.Callback());
    CompleteFuture(future);
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(&mockComputePipelineCb));
}

TEST_P(CreatePipelineAsyncAfterDeviceLossTests, CreateRenderPipelineAsync) {
    wgpu::RenderPipelineDescriptor desc = {};
    desc.label = "lost render pipeline";
    // No shader module: this descriptor would fail validation on a live device.
    EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl).Times(0);
    EXPECT_CALL(mockRenderPipelineCb,
                Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce([this](wgpu::CreatePipelineAsyncStatus, wgpu::RenderPipeline pipeline,
                         wgpu::StringView) {
            mCallbackCalled = true;
            ASSERT_NE(pipeline, nullptr);
            EXPECT_TRUE(CheckIsErrorForTesting(pipeline.Get()));
            EXPECT_EQ(FromAPI(pipeline.Get())->GetLabel(), "lost render pipeline");
        });

    wgpu::Future future =
        device.CreateRenderPipelineAsync(&desc, GetCallbackMode(), mockRenderPipelineCb.Callback());
    CompleteFuture(future);
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(&mockRenderPipelineCb));
}

// Verify CreateRenderPipelineAsync and the internal CreateRenderPipelineAsyncEvent behavior
// on creating compute pipeline with validation error.
TEST_F(CreatePipelineAsyncEventTests, InitializationValidationErrorInCreateRenderPipelineAsync) {
    wgpu::DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.depthStencil = &ds;
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));
    RenderPipelineMock* renderPipelineMockPtr = renderPipelineMock.Get();

    std::unique_ptr<UnknownError> err = DAWN_VALIDATION_ERROR(kErrorMessage);
    ON_CALL(*renderPipelineMockPtr, InitializeImpl).WillByDefault(Return(ByMove(std::move(err))));
    ON_CALL(*mDeviceMock.get(), CreateUninitializedRenderPipelineImpl)
        .WillByDefault(Return(ByMove(std::move(renderPipelineMock))));

    InSequence s;
    EXPECT_CALL(*renderPipelineMockPtr, DestroyImpl).Times(1);
    EXPECT_CALL(mockRenderPipelineCb, Call(wgpu::CreatePipelineAsyncStatus::ValidationError,
                                           IsNull(), HasSubstr(kErrorMessage)))
        .Times(1);

    device.CreateRenderPipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                     mockRenderPipelineCb.Callback());
    ProcessEvents();
}

// Verify CreateComputePipelineAsync and the internal CreateComputePipelineAsyncEvent behavior
// on creating compute pipeline with internal error.
TEST_F(CreatePipelineAsyncEventTests, InitializationUnrecoverableErrorInCreateRenderPipelineAsync) {
    wgpu::DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.depthStencil = &ds;
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));
    RenderPipelineMock* renderPipelineMockPtr = renderPipelineMock.Get();

    std::unique_ptr<UnknownError> err = DAWN_MAKE_UNRECOVERABLE_ERROR(std::string(kErrorMessage));
    ON_CALL(*renderPipelineMockPtr, InitializeImpl).WillByDefault(Return(ByMove(std::move(err))));
    ON_CALL(*mDeviceMock.get(), CreateUninitializedRenderPipelineImpl)
        .WillByDefault(Return(ByMove(std::move(renderPipelineMock))));

    InSequence s;
    EXPECT_CALL(*renderPipelineMockPtr, DestroyImpl).Times(1);
    EXPECT_CALL(mockRenderPipelineCb, Call(wgpu::CreatePipelineAsyncStatus::InternalError, IsNull(),
                                           HasSubstr(kErrorMessage)))
        .Times(1);

    device.CreateRenderPipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                     mockRenderPipelineCb.Callback());
    ProcessEvents();
}

// Test that a long async task's execution won't extend to after the device is dropped.
// Device dropping should wait for that task to finish.
TEST_F(CreatePipelineAsyncEventTests, LongAsyncTaskFinishesBeforeDeviceIsDropped) {
    wgpu::DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.depthStencil = &ds;
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    // Simulate that Initialize() would take a long time to finish.
    ON_CALL(*renderPipelineMock.Get(), InitializeImpl).WillByDefault([]() -> MaybeUnknownError {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return {};
    });
    ON_CALL(*mDeviceMock.get(), CreateUninitializedRenderPipelineImpl)
        .WillByDefault(Return(ByMove(renderPipelineMock)));

    EXPECT_CALL(mockRenderPipelineCb, Call(_, _, _)).Times(1);

    device.CreateRenderPipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                     mockRenderPipelineCb.Callback());
    ProcessEvents();
    // Dropping the device should force the async task to finish.
    DropDevice();
}

// Verify CreateComputePipelineAsync and the internal CreateComputePipelineAsyncEvent behavior
// on creating compute pipeline with validation error.
TEST_F(CreatePipelineAsyncEventTests, InitializationValidationErrorInCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    Ref<ComputePipelineMock> computePipelineMock =
        ComputePipelineMock::Create(mDeviceMock, FromCppAPI(&desc));
    ComputePipelineMock* computePipelineMockPtr = computePipelineMock.Get();

    std::unique_ptr<UnknownError> err = DAWN_VALIDATION_ERROR(kErrorMessage);
    ON_CALL(*computePipelineMockPtr, InitializeImpl).WillByDefault(Return(ByMove(std::move(err))));
    ON_CALL(*mDeviceMock.get(), CreateUninitializedComputePipelineImpl)
        .WillByDefault(Return(ByMove(std::move(computePipelineMock))));

    InSequence s;
    EXPECT_CALL(*computePipelineMockPtr, DestroyImpl).Times(1);
    EXPECT_CALL(mockComputePipelineCb, Call(wgpu::CreatePipelineAsyncStatus::ValidationError,
                                            IsNull(), HasSubstr(kErrorMessage)))
        .Times(1);

    device.CreateComputePipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                      mockComputePipelineCb.Callback());
    ProcessEvents();
}

// Verify CreateComputePipelineAsync and the internal CreateComputePipelineAsyncEvent behavior
// on creating compute pipeline with internal error.
TEST_F(CreatePipelineAsyncEventTests,
       InitializationUnrecoverableErrorInCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    Ref<ComputePipelineMock> computePipelineMock =
        ComputePipelineMock::Create(mDeviceMock, FromCppAPI(&desc));
    ComputePipelineMock* computePipelineMockPtr = computePipelineMock.Get();

    std::unique_ptr<UnknownError> err = DAWN_MAKE_UNRECOVERABLE_ERROR(std::string(kErrorMessage));
    ON_CALL(*computePipelineMockPtr, InitializeImpl).WillByDefault(Return(ByMove(std::move(err))));
    ON_CALL(*mDeviceMock.get(), CreateUninitializedComputePipelineImpl)
        .WillByDefault(Return(ByMove(std::move(computePipelineMock))));

    InSequence s;
    EXPECT_CALL(*computePipelineMockPtr, DestroyImpl).Times(1);
    EXPECT_CALL(mockComputePipelineCb, Call(wgpu::CreatePipelineAsyncStatus::InternalError,
                                            IsNull(), HasSubstr(kErrorMessage)))
        .Times(1);

    device.CreateComputePipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                      mockComputePipelineCb.Callback());
    ProcessEvents();
}

// Verify that noexcept callbacks work with CreateComputePipelineAsync.
TEST_F(CreatePipelineAsyncEventTests, NoexceptCallbackInCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    Ref<ComputePipelineMock> computePipelineMock =
        ComputePipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*computePipelineMock.Get(), InitializeImpl)
        .WillByDefault([]() -> ResultOrUnknownError<Extent3D> { return Extent3D{}; });
    ON_CALL(*mDeviceMock.get(), CreateUninitializedComputePipelineImpl)
        .WillByDefault(Return(ByMove(computePipelineMock)));

    bool called = false;
    device.CreateComputePipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [&called](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
                  wgpu::StringView) noexcept {
            called = true;
            EXPECT_EQ(status, wgpu::CreatePipelineAsyncStatus::Success);
            EXPECT_NE(pipeline, nullptr);
        });
    ProcessEvents();
    EXPECT_TRUE(called);
}

// Verify that noexcept callbacks work with CreateRenderPipelineAsync.
TEST_F(CreatePipelineAsyncEventTests, NoexceptCallbackInCreateRenderPipelineAsync) {
    wgpu::DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.depthStencil = &ds;
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*renderPipelineMock.Get(), InitializeImpl).WillByDefault([]() -> MaybeUnknownError {
        return {};
    });
    ON_CALL(*mDeviceMock.get(), CreateUninitializedRenderPipelineImpl)
        .WillByDefault(Return(ByMove(renderPipelineMock)));

    bool called = false;
    device.CreateRenderPipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [&called](wgpu::CreatePipelineAsyncStatus status, wgpu::RenderPipeline pipeline,
                  wgpu::StringView) noexcept {
            called = true;
            EXPECT_EQ(status, wgpu::CreatePipelineAsyncStatus::Success);
            EXPECT_NE(pipeline, nullptr);
        });
    ProcessEvents();
    EXPECT_TRUE(called);
}

// Verify that noexcept function pointers work with CreateComputePipelineAsync.
TEST_F(CreatePipelineAsyncEventTests, NoexceptFunctionPtrInCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    Ref<ComputePipelineMock> computePipelineMock =
        ComputePipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*computePipelineMock.Get(), InitializeImpl)
        .WillByDefault([]() -> ResultOrUnknownError<Extent3D> { return Extent3D{}; });
    ON_CALL(*mDeviceMock.get(), CreateUninitializedComputePipelineImpl)
        .WillByDefault(Return(ByMove(computePipelineMock)));

    static bool called = false;
    called = false;
    device.CreateComputePipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline, wgpu::StringView,
           bool* userdata) noexcept { *userdata = true; },
        &called);
    ProcessEvents();
    EXPECT_TRUE(called);
}

// Verify that noexcept function pointers work with CreateRenderPipelineAsync.
TEST_F(CreatePipelineAsyncEventTests, NoexceptFunctionPtrInCreateRenderPipelineAsync) {
    wgpu::DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.depthStencil = &ds;
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*renderPipelineMock.Get(), InitializeImpl).WillByDefault([]() -> MaybeUnknownError {
        return {};
    });
    ON_CALL(*mDeviceMock.get(), CreateUninitializedRenderPipelineImpl)
        .WillByDefault(Return(ByMove(renderPipelineMock)));

    static bool called = false;
    called = false;
    device.CreateRenderPipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::CreatePipelineAsyncStatus status, wgpu::RenderPipeline pipeline, wgpu::StringView,
           bool* userdata) noexcept { *userdata = true; },
        &called);
    ProcessEvents();
    EXPECT_TRUE(called);
}

}  // anonymous namespace
}  // namespace dawn::native
