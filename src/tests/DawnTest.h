// Copyright 2017 The Dawn Authors
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

#ifndef TESTS_DAWNTEST_H_
#define TESTS_DAWNTEST_H_

#include "common/Log.h"
#include "dawn/dawn_proc_table.h"
#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"

#include <gtest/gtest.h>

#include <memory>
#include <unordered_map>
#include <vector>

// Getting data back from Dawn is done in an async manners so all expectations are "deferred"
// until the end of the test. Also expectations use a copy to a MapRead buffer to get the data
// so resources should have the CopySrc allowed usage bit if you want to add expectations on
// them.
#define EXPECT_BUFFER_U32_EQ(expected, buffer, offset)                         \
    AddBufferExpectation(__FILE__, __LINE__, buffer, offset, sizeof(uint32_t), \
                         new detail::ExpectEq<uint32_t>(expected))

#define EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, offset, count)                    \
    AddBufferExpectation(__FILE__, __LINE__, buffer, offset, sizeof(uint32_t) * count, \
                         new detail::ExpectEq<uint32_t>(expected, count))

// Test a pixel of the mip level 0 of a 2D texture.
#define EXPECT_PIXEL_RGBA8_EQ(expected, texture, x, y)                                  \
    AddTextureExpectation(__FILE__, __LINE__, texture, x, y, 1, 1, 0, 0, sizeof(RGBA8), \
                          new detail::ExpectEq<RGBA8>(expected))

#define EXPECT_TEXTURE_RGBA8_EQ(expected, texture, x, y, width, height, level, slice)     \
    AddTextureExpectation(__FILE__, __LINE__, texture, x, y, width, height, level, slice, \
                          sizeof(RGBA8),                                                  \
                          new detail::ExpectEq<RGBA8>(expected, (width) * (height)))

#define EXPECT_LAZY_CLEAR(N, statement)                                                   \
    if (UsesWire()) {                                                                     \
        statement;                                                                        \
    } else {                                                                              \
        size_t lazyClearsBefore = dawn_native::GetLazyClearCountForTesting(device.Get()); \
        statement;                                                                        \
        size_t lazyClearsAfter = dawn_native::GetLazyClearCountForTesting(device.Get());  \
        EXPECT_EQ(N, lazyClearsAfter - lazyClearsBefore);                                 \
    }

// Should only be used to test validation of function that can't be tested by regular validation
// tests;
#define ASSERT_DEVICE_ERROR(statement) \
    StartExpectDeviceError();          \
    statement;                         \
    FlushWire();                       \
    ASSERT_TRUE(EndExpectDeviceError());

struct RGBA8 {
    constexpr RGBA8() : RGBA8(0, 0, 0, 0) {
    }
    constexpr RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {
    }
    bool operator==(const RGBA8& other) const;
    bool operator!=(const RGBA8& other) const;

    uint8_t r, g, b, a;

    static const RGBA8 kZero;
    static const RGBA8 kBlack;
    static const RGBA8 kRed;
    static const RGBA8 kGreen;
    static const RGBA8 kBlue;
    static const RGBA8 kYellow;
    static const RGBA8 kWhite;
};
std::ostream& operator<<(std::ostream& stream, const RGBA8& color);

struct DawnTestParam {
    explicit DawnTestParam(dawn_native::BackendType backendType) : backendType(backendType) {
    }

    dawn_native::BackendType backendType;

    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

std::ostream& operator<<(std::ostream& os, const DawnTestParam& param);

// Shorthands for backend types used in the DAWN_INSTANTIATE_TEST
extern const DawnTestParam D3D12Backend;
extern const DawnTestParam MetalBackend;
extern const DawnTestParam OpenGLBackend;
extern const DawnTestParam VulkanBackend;

DawnTestParam ForceToggles(const DawnTestParam& originParam,
                           std::initializer_list<const char*> forceEnabledWorkarounds,
                           std::initializer_list<const char*> forceDisabledWorkarounds = {});

namespace utils {
    class TerribleCommandBuffer;
}  // namespace utils

namespace detail {
    class Expectation;
}  // namespace detail

namespace dawn_wire {
    class WireClient;
    class WireServer;
}  // namespace dawn_wire

void InitDawnEnd2EndTestEnvironment(int argc, char** argv);

class DawnTestEnvironment : public testing::Environment {
  public:
    DawnTestEnvironment(int argc, char** argv);
    ~DawnTestEnvironment() = default;

    static void SetEnvironment(DawnTestEnvironment* env);

    void SetUp() override;
    void TearDown() override;

    bool UsesWire() const;
    bool IsBackendValidationEnabled() const;
    bool IsDawnValidationSkipped() const;
    bool IsSpvcBeingUsed() const;
    dawn_native::Instance* GetInstance() const;
    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;

  protected:
    std::unique_ptr<dawn_native::Instance> mInstance;

  private:
    void DiscoverOpenGLAdapter();

    bool mUseWire = false;
    bool mEnableBackendValidation = false;
    bool mSkipDawnValidation = false;
    bool mUseSpvc = false;
    bool mBeginCaptureOnStartup = false;
    bool mHasVendorIdFilter = false;
    uint32_t mVendorIdFilter = 0;
};

class DawnTestBase {
    friend class DawnPerfTestBase;

  public:
    DawnTestBase(const DawnTestParam& param);
    virtual ~DawnTestBase();

    void SetUp();
    void TearDown();

    bool IsD3D12() const;
    bool IsMetal() const;
    bool IsOpenGL() const;
    bool IsVulkan() const;

    bool IsAMD() const;
    bool IsARM() const;
    bool IsImgTec() const;
    bool IsIntel() const;
    bool IsNvidia() const;
    bool IsQualcomm() const;

    bool IsWindows() const;
    bool IsLinux() const;
    bool IsMacOS() const;

    bool UsesWire() const;
    bool IsBackendValidationEnabled() const;
    bool IsDawnValidationSkipped() const;
    bool IsSpvcBeingUsed() const;

    void StartExpectDeviceError();
    bool EndExpectDeviceError();

    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;

    dawn_native::PCIInfo GetPCIInfo() const;

  protected:
    wgpu::Device device;
    wgpu::Queue queue;

    DawnProcTable backendProcs = {};
    WGPUDevice backendDevice = nullptr;

    // Helper methods to implement the EXPECT_ macros
    std::ostringstream& AddBufferExpectation(const char* file,
                                             int line,
                                             const wgpu::Buffer& buffer,
                                             uint64_t offset,
                                             uint64_t size,
                                             detail::Expectation* expectation);
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const wgpu::Texture& texture,
                                              uint32_t x,
                                              uint32_t y,
                                              uint32_t width,
                                              uint32_t height,
                                              uint32_t level,
                                              uint32_t slice,
                                              uint32_t pixelSize,
                                              detail::Expectation* expectation);

    bool HasAdapter() const;
    void WaitABit();
    void FlushWire();

    bool SupportsExtensions(const std::vector<const char*>& extensions);

    // Called in SetUp() to get the extensions required to be enabled in the tests. The tests must
    // check if the required extensions are supported by the adapter in this function and guarantee
    // the returned extensions are all supported by the adapter. The tests may provide different
    // code path to handle the situation when not all extensions are supported.
    virtual std::vector<const char*> GetRequiredExtensions();

  private:
    DawnTestParam mParam;

    // Things used to set up testing through the Wire.
    std::unique_ptr<dawn_wire::WireServer> mWireServer;
    std::unique_ptr<dawn_wire::WireClient> mWireClient;
    std::unique_ptr<utils::TerribleCommandBuffer> mC2sBuf;
    std::unique_ptr<utils::TerribleCommandBuffer> mS2cBuf;

    // Tracking for validation errors
    static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata);
    bool mExpectError = false;
    bool mError = false;

    // MapRead buffers used to get data for the expectations
    struct ReadbackSlot {
        wgpu::Buffer buffer;
        uint64_t bufferSize;
        const void* mappedData = nullptr;
    };
    std::vector<ReadbackSlot> mReadbackSlots;

    // Maps all the buffers and fill ReadbackSlot::mappedData
    void MapSlotsSynchronously();
    static void SlotMapReadCallback(WGPUBufferMapAsyncStatus status,
                                    const void* data,
                                    uint64_t dataLength,
                                    void* userdata);
    size_t mNumPendingMapOperations = 0;

    // Reserve space where the data for an expectation can be copied
    struct ReadbackReservation {
        wgpu::Buffer buffer;
        size_t slot;
        uint64_t offset;
    };
    ReadbackReservation ReserveReadback(uint64_t readbackSize);

    struct DeferredExpectation {
        const char* file;
        int line;
        size_t readbackSlot;
        uint64_t readbackOffset;
        uint64_t size;
        uint32_t rowBytes;
        uint32_t rowPitch;
        std::unique_ptr<detail::Expectation> expectation;
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
        // Use unique_ptr because of missing move/copy constructors on std::basic_ostringstream
        std::unique_ptr<std::ostringstream> message;
    };
    std::vector<DeferredExpectation> mDeferredExpectations;

    // Assuming the data is mapped, checks all expectations
    void ResolveExpectations();

    dawn_native::PCIInfo mPCIInfo;

    dawn_native::Adapter mBackendAdapter;
};

// Skip a test when the given condition is satisfied.
#define DAWN_SKIP_TEST_IF(condition)                  \
    if (condition) {                                  \
        InfoLog() << "Test skipped: " #condition "."; \
        GTEST_SKIP();                                 \
        return;                                       \
    }

template <typename Params = DawnTestParam>
class DawnTestWithParams : public DawnTestBase, public ::testing::TestWithParam<Params> {
  private:
    void SetUp() override final {
        // DawnTestBase::SetUp() gets the adapter, and creates the device and wire.
        // It's separate from TestSetUp() so we can skip tests completely if no adapter
        // is available.
        DawnTestBase::SetUp();
        DAWN_SKIP_TEST_IF(!HasAdapter());
        TestSetUp();
    }

  protected:
    DawnTestWithParams();
    ~DawnTestWithParams() override = default;

    virtual void TestSetUp() {
    }

    void TearDown() override {
        DawnTestBase::TearDown();
    }
};

template <typename Params>
DawnTestWithParams<Params>::DawnTestWithParams() : DawnTestBase(this->GetParam()) {
}

using DawnTest = DawnTestWithParams<>;

// Instantiate the test once for each backend provided after the first argument. Use it like this:
//     DAWN_INSTANTIATE_TEST(MyTestFixture, MetalBackend, OpenGLBackend)
#define DAWN_INSTANTIATE_TEST(testName, firstParam, ...)                         \
    const decltype(firstParam) testName##params[] = {firstParam, ##__VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(                                                    \
        , testName,                                                              \
        testing::ValuesIn(::detail::FilterBackends(                              \
            testName##params, sizeof(testName##params) / sizeof(firstParam))),   \
        testing::PrintToStringParamName())


namespace detail {
    // Helper functions used for DAWN_INSTANTIATE_TEST
    bool IsBackendAvailable(dawn_native::BackendType type);
    std::vector<DawnTestParam> FilterBackends(const DawnTestParam* params, size_t numParams);

    // All classes used to implement the deferred expectations should inherit from this.
    class Expectation {
      public:
        virtual ~Expectation() = default;

        // Will be called with the buffer or texture data the expectation should check.
        virtual testing::AssertionResult Check(const void* data, size_t size) = 0;
    };

    // Expectation that checks the data is equal to some expected values.
    template <typename T>
    class ExpectEq : public Expectation {
      public:
        ExpectEq(T singleValue);
        ExpectEq(const T* values, const unsigned int count);

        testing::AssertionResult Check(const void* data, size_t size) override;

      private:
        std::vector<T> mExpected;
    };
    extern template class ExpectEq<uint8_t>;
    extern template class ExpectEq<uint32_t>;
    extern template class ExpectEq<RGBA8>;
}  // namespace detail

#endif  // TESTS_DAWNTEST_H_
