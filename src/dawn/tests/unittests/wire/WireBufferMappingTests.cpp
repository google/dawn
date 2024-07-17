// Copyright 2019 The Dawn & Tint Authors
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

#include <limits>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

// Define a stream operator for WGPUMapMode outside namespace scope so that it can be found on
// resolution for test name generation.
// TODO(dawn:2205) Remove this in favor of custom serializer.
std::ostream& operator<<(std::ostream& os, const WGPUMapMode& param) {
    switch (param) {
        case WGPUMapMode_Read:
            os << "Read";
            break;
        case WGPUMapMode_Write:
            os << "Write";
            break;
        default:
            DAWN_UNREACHABLE();
    }
    return os;
}

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Return;

// For the buffer tests, we make passing a map mode optional to reuse the same test fixture for
// tests that test multiple modes and tests that are mode specific. By making it an optional, it
// allows us to determine whether the map mode is necessary when generating the test names.
using MapMode = std::optional<WGPUMapMode>;
DAWN_WIRE_FUTURE_TEST_PARAM_STRUCT(WireBufferParam, MapMode);

using WireBufferMappingTestBase = WireFutureTestWithParams<WGPUBufferMapCallback,
                                                           WGPUBufferMapCallbackInfo,
                                                           wgpuBufferMapAsync,
                                                           wgpuBufferMapAsyncF,
                                                           WireBufferParam>;

// General mapping tests that either do not care about the specific mapping mode, or apply to both.
class WireBufferMappingTests : public WireBufferMappingTestBase {
  protected:
    void SetUp() override {
        WireBufferMappingTestBase::SetUp();
        apiBuffer = api.GetNewBuffer();
    }

    // Overridden version of wgpuBufferMapAsync that defers to the API call based on the
    // test callback mode.
    void BufferMapAsync(WGPUBuffer b,
                        WGPUMapMode mode,
                        size_t offset,
                        size_t size,
                        void* userdata = nullptr) {
        CallImpl(userdata, b, mode, offset, size);
    }

    WGPUMapMode GetMapMode() {
        DAWN_ASSERT(GetParam().mMapMode);
        return *GetParam().mMapMode;
    }

    void SetupBuffer(WGPUMapMode mapMode) {
        WGPUBufferUsage usage = WGPUBufferUsage_MapRead;
        if (mapMode == WGPUMapMode_Read) {
            usage = WGPUBufferUsage_MapRead;
        } else if (mapMode == WGPUMapMode_Write) {
            usage = WGPUBufferUsage_MapWrite;
        }

        WGPUBufferDescriptor descriptor = {};
        descriptor.size = kBufferSize;
        descriptor.usage = usage;

        buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
            .WillOnce(Return(apiBuffer))
            .RetiresOnSaturation();
        FlushClient();
    }

    // Sets up the correct mapped range call expectations given the map mode.
    void ExpectMappedRangeCall(uint64_t bufferSize, void* bufferContent) {
        WGPUMapMode mapMode = GetMapMode();
        if (mapMode == WGPUMapMode_Read) {
            EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, bufferSize))
                .WillOnce(Return(bufferContent));
        } else if (mapMode == WGPUMapMode_Write) {
            EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, bufferSize))
                .WillOnce(Return(bufferContent));
        }
    }

    // Test to exercise client functions that should override server response for callbacks.
    template <typename CancelFn, typename ExpFn>
    void TestEarlyMapCancelled(CancelFn cancelMapping,
                               ExpFn addExpectations,
                               WGPUBufferMapAsyncStatus expected,
                               bool calledInCancelFn) {
        WGPUMapMode mapMode = GetMapMode();
        SetupBuffer(mapMode);
        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

        uint32_t bufferContent = 31337;
        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
            }));
        ExpectMappedRangeCall(kBufferSize, &bufferContent);
        addExpectations();

        // The callback should get called with the expected status, regardless if the server has
        // responded.
        if (calledInCancelFn) {
            // In spontaneous mode, the callback gets called as a part of the cancel function.
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, _)).Times(1);

                cancelMapping();
            });
            FlushClient();
            FlushCallbacks();
        } else {
            // Otherwise, the callback will fire when we flush them.
            cancelMapping();
            FlushClient();
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, _)).Times(1);

                FlushCallbacks();
            });
        }
    }

    // Test to exercise client functions that should override server error response for callbacks.
    template <typename CancelFn, typename ExpFn>
    void TestEarlyMapErrorCancelled(CancelFn cancelMapping,
                                    ExpFn addExpectations,
                                    WGPUBufferMapAsyncStatus expected,
                                    bool calledInCancelFn) {
        WGPUMapMode mapMode = GetMapMode();
        SetupBuffer(mapMode);
        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
            }));

        // Ensure that the server had a chance to respond if relevant.
        FlushClient();
        FlushFutures();

        addExpectations();

        // The callback should get called with the expected status status, not server-side error,
        // even if the request fails on the server side.
        if (calledInCancelFn) {
            // In spontaneous mode, the callback gets called as a part of the cancel function.
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, _)).Times(1);

                cancelMapping();
            });
            FlushClient();
            FlushCallbacks();
        } else {
            // Otherwise, the callback will fire when we flush them.
            cancelMapping();
            FlushClient();
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, _)).Times(1);

                FlushCallbacks();
            });
        }
    }

    // Test to exercise client functions that would cancel callbacks don't cause the callback to be
    // fired twice.
    template <typename ExpFn>
    void TestCancelInCallback(void (*cancelFn)(WGPUBuffer), ExpFn cancelExp) {
        WGPUMapMode mapMode = GetMapMode();
        SetupBuffer(mapMode);
        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

        uint32_t bufferContent = 31337;
        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
            }));
        ExpectMappedRangeCall(kBufferSize, &bufferContent);

        // Ensure that the server had a chance to respond if relevant.
        FlushClient();
        FlushFutures();

        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).WillOnce([&]() {
                cancelFn(buffer);
            });

            FlushCallbacks();
        });

        // Make sure that the cancel function is called and flush more callbacks to ensure that
        // nothing else happens.
        cancelExp();
        FlushClient();
        FlushFutures();
        FlushCallbacks();
    }

    static constexpr uint64_t kBufferSize = sizeof(uint32_t);
    // A successfully created buffer
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappingTests, {WGPUMapMode_Read, WGPUMapMode_Write});

// Check that things work correctly when a validation error happens when mapping the buffer.
TEST_P(WireBufferMappingTests, ErrorWhileMapping) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);
    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

        FlushCallbacks();
    });

    EXPECT_EQ(nullptr, wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize));
}

// Check the map callback is called with "UnmappedBeforeCallback" when the map request would have
// worked, but Unmap() was called.
TEST_P(WireBufferMappingTests, UnmapCalledTooEarly) {
    TestEarlyMapCancelled([&]() { wgpuBufferUnmap(buffer); },
                          [&]() { EXPECT_CALL(api, BufferUnmap(apiBuffer)); },
                          WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, IsSpontaneous());
}

// Check that if Unmap() was called early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, UnmapCalledTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled([&]() { wgpuBufferUnmap(buffer); },
                               [&]() { EXPECT_CALL(api, BufferUnmap(apiBuffer)); },
                               WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, IsSpontaneous());
}

// Check the map callback is called with "DestroyedBeforeCallback" when the map request would have
// worked, but Destroy() was called.
TEST_P(WireBufferMappingTests, DestroyCalledTooEarly) {
    TestEarlyMapCancelled([&]() { wgpuBufferDestroy(buffer); },
                          [&]() { EXPECT_CALL(api, BufferDestroy(apiBuffer)); },
                          WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, IsSpontaneous());
}

// Check that if Destroy() was called early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, DestroyCalledTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled([&]() { wgpuBufferDestroy(buffer); },
                               [&]() { EXPECT_CALL(api, BufferDestroy(apiBuffer)); },
                               WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, IsSpontaneous());
}

// Check the map callback is called with "DestroyedBeforeCallback" when the map request would have
// worked, but the device was released.
TEST_P(WireBufferMappingTests, DeviceReleasedTooEarly) {
    TestEarlyMapCancelled(
        [&]() { device = nullptr; },
        [&]() {
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
            EXPECT_CALL(api, DeviceRelease(apiDevice));
        },
        WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, false);
    DefaultApiDeviceWasReleased();
}

// Check that if device is released early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, DeviceReleasedTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled(
        [&]() { device = nullptr; },
        [&]() {
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
            EXPECT_CALL(api, DeviceRelease(apiDevice));
        },
        WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, false);
    DefaultApiDeviceWasReleased();
}

// Check the map callback is called with "DestroyedBeforeCallback" when the map request would have
// worked, but the device was destroyed.
TEST_P(WireBufferMappingTests, DeviceDestroyedTooEarly) {
    TestEarlyMapCancelled([&]() { wgpuDeviceDestroy(cDevice); },
                          [&]() { EXPECT_CALL(api, DeviceDestroy(apiDevice)); },
                          WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, false);
}

// Check that if device is destroyed early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, DeviceDestroyedTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled([&]() { wgpuDeviceDestroy(cDevice); },
                               [&]() { EXPECT_CALL(api, DeviceDestroy(apiDevice)); },
                               WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, false);
}

// Test that the callback isn't fired twice when Unmap() is called inside the callback.
TEST_P(WireBufferMappingTests, UnmapInsideMapCallback) {
    TestCancelInCallback(&wgpuBufferUnmap, [&]() { EXPECT_CALL(api, BufferUnmap(apiBuffer)); });
}

// Test that the callback isn't fired twice when Destroy() is called inside the callback.
TEST_P(WireBufferMappingTests, DestroyInsideMapCallback) {
    TestCancelInCallback(&wgpuBufferDestroy, [&]() { EXPECT_CALL(api, BufferDestroy(apiBuffer)); });
}

// Test that the callback isn't fired twice when Release() is called inside the callback with the
// last ref.
TEST_P(WireBufferMappingTests, ReleaseInsideMapCallback) {
    // TODO(dawn:1621): Suppressed because the mapping handling still touches the buffer after it is
    // destroyed triggering an ASAN error when in MapWrite mode.
    DAWN_SKIP_TEST_IF(GetMapMode() == WGPUMapMode_Write);

    TestCancelInCallback(&wgpuBufferRelease, [&]() { EXPECT_CALL(api, BufferRelease(apiBuffer)); });
}

// Tests specific to mapping for reading.
class WireBufferMappingReadTests : public WireBufferMappingTests {
  protected:
    void SetUp() override {
        WireBufferMappingTests::SetUp();
        SetupBuffer(WGPUMapMode_Read);
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappingReadTests);

// Check mapping for reading a succesfully created buffer.
TEST_P(WireBufferMappingReadTests, MappingSuccess) {
    BufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

        FlushCallbacks();
    });

    EXPECT_EQ(bufferContent,
              *static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));
    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    FlushClient();
}

// Check that an error map read while a buffer is already mapped won't changed the result of get
// mapped range.
TEST_P(WireBufferMappingReadTests, MappingErrorWhileAlreadyMapped) {
    // Successful map
    BufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

        FlushCallbacks();
    });

    // Map failure while the buffer is already mapped
    BufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

        FlushCallbacks();
    });

    EXPECT_EQ(bufferContent,
              *static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));
}

// Tests specific to mapping for writing.
class WireBufferMappingWriteTests : public WireBufferMappingTests {
  protected:
    void SetUp() override {
        WireBufferMappingTests::SetUp();
        SetupBuffer(WGPUMapMode_Write);
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappingWriteTests);

// Check mapping for writing a succesfully created buffer.
TEST_P(WireBufferMappingWriteTests, MappingSuccess) {
    BufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, nullptr);

    uint32_t serverBufferContent = 31337;
    uint32_t updatedContent = 4242;

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&serverBufferContent));

    // The map write callback always gets a buffer full of zeroes.
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

        FlushCallbacks();
    });

    uint32_t* lastMapWritePointer =
        static_cast<uint32_t*>(wgpuBufferGetMappedRange(buffer, 0, kBufferSize));
    ASSERT_EQ(0u, *lastMapWritePointer);

    // Write something to the mapped pointer
    *lastMapWritePointer = updatedContent;

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // After the buffer is unmapped, the content of the buffer is updated on the server
    ASSERT_EQ(serverBufferContent, updatedContent);
}

// Check that an error map write while a buffer is already mapped.
TEST_P(WireBufferMappingWriteTests, MappingErrorWhileAlreadyMapped) {
    // Successful map
    BufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

        FlushCallbacks();
    });

    // Map failure while the buffer is already mapped
    BufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, nullptr);
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

        FlushCallbacks();
    });

    EXPECT_NE(nullptr,
              static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));
}

// Tests specific to mapped at creation.
class WireBufferMappedAtCreationTests : public WireBufferMappingTests {};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappedAtCreationTests);

// Test successful buffer creation with mappedAtCreation=true
TEST_F(WireBufferMappedAtCreationTests, Success) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Test that releasing a buffer mapped at creation does not call Unmap
TEST_F(WireBufferMappedAtCreationTests, ReleaseBeforeUnmap) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferRelease(buffer);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);

    FlushClient();
}

// Test that it is valid to map a buffer after it is mapped at creation and unmapped.
TEST_P(WireBufferMappedAtCreationTests, MapSuccess) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.usage = WGPUMapMode_Write;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    BufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&apiBufferData));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

        FlushCallbacks();
    });
}

// Test that it is invalid to map a buffer after mappedAtCreation but before Unmap
TEST_P(WireBufferMappedAtCreationTests, MapFailure) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    BufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, nullptr);

    // Note that the validation logic is entirely on the native side so we inject the validation
    // error here and flush the server response to mock the expected behavior.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

        FlushCallbacks();
    });

    EXPECT_NE(nullptr,
              static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Check that trying to create a buffer of size MAX_SIZE_T won't get OOM error at the client side.
TEST_F(WireBufferMappingTests, MaxSizeMappableBufferOOMDirectly) {
    size_t kOOMSize = std::numeric_limits<size_t>::max();
    WGPUBuffer apiBuffer = api.GetNewBuffer();

    // Check for CreateBufferMapped.
    {
        WGPUBufferDescriptor descriptor = {};
        descriptor.usage = WGPUBufferUsage_CopySrc;
        descriptor.size = kOOMSize;
        descriptor.mappedAtCreation = true;

        wgpuDeviceCreateBuffer(cDevice, &descriptor);
        FlushClient();
    }

    // Check for MapRead usage.
    {
        WGPUBufferDescriptor descriptor = {};
        descriptor.usage = WGPUBufferUsage_MapRead;
        descriptor.size = kOOMSize;

        wgpuDeviceCreateBuffer(cDevice, &descriptor);
        EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        FlushClient();
    }

    // Check for MapWrite usage.
    {
        WGPUBufferDescriptor descriptor = {};
        descriptor.usage = WGPUBufferUsage_MapWrite;
        descriptor.size = kOOMSize;

        wgpuDeviceCreateBuffer(cDevice, &descriptor);
        EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        FlushClient();
    }
}

// Test that registering a callback then wire disconnect calls the callback with
// DeviceLost.
TEST_P(WireBufferMappingTests, MapThenDisconnect) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);
    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

    uint32_t bufferContent = 0;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    FlushClient();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_DeviceLost, _)).Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test that registering a callback after wire disconnect calls the callback with
// DeviceLost.
TEST_P(WireBufferMappingTests, MapAfterDisconnect) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);

    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_DeviceLost, _)).Times(1);

        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);
    });
}

// Test that mapping again while pending map cause an error on the callback.
TEST_P(WireBufferMappingTests, PendingMapImmediateError) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);
    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

    // Calls for the first successful map.
    uint32_t bufferContent = 0;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    if (IsSpontaneous()) {
        // In spontaneous mode, the second map on the pending immediately calls the callback.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, _)).Times(1);

            BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);
        });

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

            FlushCallbacks();
        });
    } else {
        // Otherwise, the callback will fire alongside the success one when we flush the callbacks.
        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, _)).Times(1);
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

            FlushCallbacks();
        });
    }
}

// Test that GetMapState() returns map state as expected
TEST_P(WireBufferMappingTests, GetMapState) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);

    uint32_t bufferContent = 31337;
    // Server-side success case
    {
        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
            }));
        ExpectMappedRangeCall(kBufferSize, &bufferContent);

        // Map state should initially be unmapped.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

        // Map state should become pending immediately after map async call.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushClient();

        // Map state should be pending until receiving a response from server.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushFutures();

        // Map state should still be pending until the callback has been called.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).WillOnce([&]() {
                ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Mapped);
            });

            FlushCallbacks();
        });

        // Mapping succeeded.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Mapped);
    }

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    FlushClient();

    // Server-side error case
    {
        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
            }));

        // Map state should initially be unmapped.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
        BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

        // Map state should become pending immediately after map async call.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushClient();

        // Map state should be pending until receiving a response from server.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushFutures();

        // Map state should still be pending until the callback has been called.
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).WillOnce([&]() {
                ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
            });

            FlushCallbacks();
        });

        // mapping failed
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
    }
}

// Test that requests inside user callbacks before disconnect are called.
TEST_P(WireBufferMappingTests, MapInsideCallbackBeforeDisconnect) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);
    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

    uint32_t bufferContent = 0;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    FlushClient();

    static constexpr size_t kNumRequests = 10;
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_DeviceLost, _))
            .Times(kNumRequests + 1)
            .WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);
                }
            })
            .WillRepeatedly(Return());

        GetWireClient()->Disconnect();
    });
}

// Test that requests inside user callbacks before buffer destroy are called.
TEST_P(WireBufferMappingTests, MapInsideCallbackBeforeDestroy) {
    WGPUMapMode mapMode = GetMapMode();
    SetupBuffer(mapMode);
    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);

    uint32_t bufferContent = 0;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, mapMode, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    FlushClient();
    FlushFutures();

    static constexpr size_t kNumRequests = 10;
    if (IsSpontaneous()) {
        // In spontaneous mode, when the success callback fires, the first MapAsync request
        // generated by the callback is queued, then all subsequent requests' callbacks are
        // immediately called with MappingAlreadyPending. Finally, when we call Destroy, the queued
        // request's callback is then called with DestroyedBeforeCallback.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);
                }
            });
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, _))
                .Times(kNumRequests - 1);

            FlushCallbacks();
        });
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _)).Times(1);

            wgpuBufferDestroy(buffer);
        });
        FlushCallbacks();
    } else {
        // In non-spontaneous modes, the first callback doesn't trigger any other immediate
        // callbacks, but internally, all but the first MapAsync call's callback is set to be ready
        // with MappingAlreadyPending. When we call Destroy, the first pending request is then
        // marked ready with DestroyedBeforeCallback. The callbacks all run when we flush them.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_Success, _)).WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    BufferMapAsync(buffer, mapMode, 0, kBufferSize, nullptr);
                }
            });

            FlushCallbacks();
        });
        wgpuBufferDestroy(buffer);
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, _))
                .Times(kNumRequests - 1);
            EXPECT_CALL(mockCb, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _)).Times(1);

            FlushCallbacks();
        });
    }
}

}  // anonymous namespace
}  // namespace dawn::wire
