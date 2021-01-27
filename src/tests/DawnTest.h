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

#include <dawn_platform/DawnPlatform.h>
#include <gtest/gtest.h>

#include <memory>
#include <unordered_map>
#include <vector>

// Getting data back from Dawn is done in an async manners so all expectations are "deferred"
// until the end of the test. Also expectations use a copy to a MapRead buffer to get the data
// so resources should have the CopySrc allowed usage bit if you want to add expectations on
// them.

#define EXPECT_BUFFER(buffer, offset, size, expectation) \
    AddBufferExpectation(__FILE__, __LINE__, buffer, offset, size, expectation)

#define EXPECT_BUFFER_U16_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint16_t), new ::detail::ExpectEq<uint16_t>(expected))

#define EXPECT_BUFFER_U16_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint16_t) * (count),       \
                  new ::detail::ExpectEq<uint16_t>(expected, count))

#define EXPECT_BUFFER_U32_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint32_t), new ::detail::ExpectEq<uint32_t>(expected))

#define EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint32_t) * (count),       \
                  new ::detail::ExpectEq<uint32_t>(expected, count))

#define EXPECT_BUFFER_U64_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint64_t), new ::detail::ExpectEq<uint64_t>(expected))

#define EXPECT_BUFFER_U64_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint64_t) * (count),       \
                  new ::detail::ExpectEq<uint64_t>(expected, count))

#define EXPECT_BUFFER_FLOAT_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(float), new ::detail::ExpectEq<float>(expected))

#define EXPECT_BUFFER_FLOAT_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(float) * (count),            \
                  new ::detail::ExpectEq<float>(expected, count))

// Test a pixel of the mip level 0 of a 2D texture.
#define EXPECT_PIXEL_RGBA8_EQ(expected, texture, x, y) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, x, y)

#define EXPECT_TEXTURE_RGBA8_EQ(expected, texture, x, y, width, height, level, slice) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, x, y, width, height, level, slice)

#define EXPECT_PIXEL_FLOAT_EQ(expected, texture, x, y) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, x, y)

#define EXPECT_TEXTURE_FLOAT_EQ(expected, texture, x, y, width, height, level, slice) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, x, y, width, height, level, slice)

#define EXPECT_PIXEL_RGBA8_BETWEEN(color0, color1, texture, x, y) \
    AddTextureBetweenColorsExpectation(__FILE__, __LINE__, color0, color1, texture, x, y)

// TODO(enga): Migrate other texure expectation helpers to this common one.
#define EXPECT_TEXTURE_EQ(...) AddTextureExpectation(__FILE__, __LINE__, __VA_ARGS__)

// Should only be used to test validation of function that can't be tested by regular validation
// tests;
#define ASSERT_DEVICE_ERROR(statement)                          \
    StartExpectDeviceError();                                   \
    statement;                                                  \
    FlushWire();                                                \
    if (!EndExpectDeviceError()) {                              \
        FAIL() << "Expected device error in:\n " << #statement; \
    }                                                           \
    do {                                                        \
    } while (0)

struct RGBA8 {
    constexpr RGBA8() : RGBA8(0, 0, 0, 0) {
    }
    constexpr RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {
    }
    bool operator==(const RGBA8& other) const;
    bool operator!=(const RGBA8& other) const;
    bool operator<=(const RGBA8& other) const;
    bool operator>=(const RGBA8& other) const;

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

struct BackendTestConfig {
    BackendTestConfig(wgpu::BackendType backendType,
                      std::initializer_list<const char*> forceEnabledWorkarounds = {},
                      std::initializer_list<const char*> forceDisabledWorkarounds = {});

    wgpu::BackendType backendType;

    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

struct TestAdapterProperties : wgpu::AdapterProperties {
    TestAdapterProperties(const wgpu::AdapterProperties& properties, bool selected);
    std::string adapterName;
    bool selected;

  private:
    // This may be temporary, so it is copied into |adapterName| and made private.
    using wgpu::AdapterProperties::name;
};

struct AdapterTestParam {
    AdapterTestParam(const BackendTestConfig& config,
                     const TestAdapterProperties& adapterProperties);

    TestAdapterProperties adapterProperties;
    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

std::ostream& operator<<(std::ostream& os, const AdapterTestParam& param);

BackendTestConfig D3D12Backend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig MetalBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig NullBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                              std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig OpenGLBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig OpenGLESBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                  std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig VulkanBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                std::initializer_list<const char*> forceDisabledWorkarounds = {});

struct GLFWwindow;

namespace utils {
    class PlatformDebugLogger;
    class TerribleCommandBuffer;
    class WireHelper;
}  // namespace utils

namespace detail {
    class Expectation;

    template <typename T>
    class ExpectEq;
    template <typename T>
    class ExpectBetweenColors;
}  // namespace detail

namespace dawn_wire {
    class CommandHandler;
    class WireClient;
    class WireServer;
}  // namespace dawn_wire

void InitDawnEnd2EndTestEnvironment(int argc, char** argv);

class DawnTestEnvironment : public testing::Environment {
  public:
    DawnTestEnvironment(int argc, char** argv);
    ~DawnTestEnvironment() override;

    static void SetEnvironment(DawnTestEnvironment* env);

    std::vector<AdapterTestParam> GetAvailableAdapterTestParamsForBackends(
        const BackendTestConfig* params,
        size_t numParams);

    void SetUp() override;
    void TearDown() override;

    bool UsesWire() const;
    bool IsBackendValidationEnabled() const;
    dawn_native::Instance* GetInstance() const;
    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;
    const char* GetWireTraceDir() const;
    GLFWwindow* GetOpenGLWindow() const;
    GLFWwindow* GetOpenGLESWindow() const;

    const std::vector<std::string>& GetEnabledToggles() const;
    const std::vector<std::string>& GetDisabledToggles() const;

  protected:
    std::unique_ptr<dawn_native::Instance> mInstance;

  private:
    void ParseArgs(int argc, char** argv);
    std::unique_ptr<dawn_native::Instance> CreateInstanceAndDiscoverAdapters();
    void SelectPreferredAdapterProperties(const dawn_native::Instance* instance);
    void PrintTestConfigurationAndAdapterInfo(dawn_native::Instance* instance) const;

    bool mUseWire = false;
    bool mEnableBackendValidation = false;
    bool mBeginCaptureOnStartup = false;
    bool mHasVendorIdFilter = false;
    uint32_t mVendorIdFilter = 0;
    std::string mWireTraceDir;

    std::vector<std::string> mEnabledToggles;
    std::vector<std::string> mDisabledToggles;
    std::vector<dawn_native::DeviceType> mDevicePreferences;
    std::vector<TestAdapterProperties> mAdapterProperties;

    std::unique_ptr<utils::PlatformDebugLogger> mPlatformDebugLogger;
    GLFWwindow* mOpenGLWindow;
    GLFWwindow* mOpenGLESWindow;
};

class DawnTestBase {
    friend class DawnPerfTestBase;

  public:
    DawnTestBase(const AdapterTestParam& param);
    virtual ~DawnTestBase();

    void SetUp();
    void TearDown();

    bool IsD3D12() const;
    bool IsMetal() const;
    bool IsNull() const;
    bool IsOpenGL() const;
    bool IsOpenGLES() const;
    bool IsVulkan() const;

    bool IsAMD() const;
    bool IsARM() const;
    bool IsImgTec() const;
    bool IsIntel() const;
    bool IsNvidia() const;
    bool IsQualcomm() const;
    bool IsSwiftshader() const;
    bool IsANGLE() const;
    bool IsWARP() const;

    bool IsWindows() const;
    bool IsLinux() const;
    bool IsMacOS() const;

    bool UsesWire() const;
    bool IsBackendValidationEnabled() const;
    bool HasWGSL() const;

    bool IsAsan() const;

    bool HasToggleEnabled(const char* workaround) const;

    void StartExpectDeviceError();
    bool EndExpectDeviceError();

    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;

    wgpu::Instance GetInstance() const;
    dawn_native::Adapter GetAdapter() const;

    virtual std::unique_ptr<dawn_platform::Platform> CreateTestPlatform();

  protected:
    wgpu::Device device;
    wgpu::Queue queue;

    DawnProcTable backendProcs = {};
    WGPUDevice backendDevice = nullptr;

    size_t mLastWarningCount = 0;

    // Helper methods to implement the EXPECT_ macros
    std::ostringstream& AddBufferExpectation(const char* file,
                                             int line,
                                             const wgpu::Buffer& buffer,
                                             uint64_t offset,
                                             uint64_t size,
                                             detail::Expectation* expectation);

    template <typename T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              uint32_t x,
                                              uint32_t y,
                                              uint32_t width = 1,
                                              uint32_t height = 1,
                                              uint32_t level = 0,
                                              uint32_t slice = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(
            file, line, new detail::ExpectEq<T>(expectedData, width * height), texture, x, y, width,
            height, level, slice, aspect, sizeof(T), bytesPerRow);
    }

    template <typename T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T& expectedData,
                                              const wgpu::Texture& texture,
                                              uint32_t x,
                                              uint32_t y,
                                              uint32_t level = 0,
                                              uint32_t slice = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(file, line, new detail::ExpectEq<T>(expectedData), texture,
                                         x, y, 1, 1, level, slice, aspect, sizeof(T), bytesPerRow);
    }

    template <typename T>
    std::ostringstream& AddTextureBetweenColorsExpectation(
        const char* file,
        int line,
        const T& color0,
        const T& color1,
        const wgpu::Texture& texture,
        uint32_t x,
        uint32_t y,
        uint32_t level = 0,
        uint32_t slice = 0,
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
        uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(
            file, line, new detail::ExpectBetweenColors<T>(color0, color1), texture, x, y, 1, 1,
            level, slice, aspect, sizeof(T), bytesPerRow);
    }

    void WaitABit();
    void FlushWire();
    void WaitForAllOperations();

    bool SupportsExtensions(const std::vector<const char*>& extensions);

    // Called in SetUp() to get the extensions required to be enabled in the tests. The tests must
    // check if the required extensions are supported by the adapter in this function and guarantee
    // the returned extensions are all supported by the adapter. The tests may provide different
    // code path to handle the situation when not all extensions are supported.
    virtual std::vector<const char*> GetRequiredExtensions();

    const wgpu::AdapterProperties& GetAdapterProperties() const;

  private:
    AdapterTestParam mParam;
    std::unique_ptr<utils::WireHelper> mWireHelper;

    // Tracking for validation errors
    static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata);
    static void OnDeviceLost(const char* message, void* userdata);
    bool mExpectError = false;
    bool mError = false;

    std::ostringstream& AddTextureExpectationImpl(const char* file,
                                                  int line,
                                                  detail::Expectation* expectation,
                                                  const wgpu::Texture& texture,
                                                  uint32_t x,
                                                  uint32_t y,
                                                  uint32_t width,
                                                  uint32_t height,
                                                  uint32_t level,
                                                  uint32_t slice,
                                                  wgpu::TextureAspect aspect,
                                                  uint32_t dataSize,
                                                  uint32_t bytesPerRow);

    // MapRead buffers used to get data for the expectations
    struct ReadbackSlot {
        wgpu::Buffer buffer;
        uint64_t bufferSize;
        const void* mappedData = nullptr;
    };
    std::vector<ReadbackSlot> mReadbackSlots;

    // Maps all the buffers and fill ReadbackSlot::mappedData
    void MapSlotsSynchronously();
    static void SlotMapCallback(WGPUBufferMapAsyncStatus status, void* userdata);
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
        uint32_t bytesPerRow;
        std::unique_ptr<detail::Expectation> expectation;
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
        // Use unique_ptr because of missing move/copy constructors on std::basic_ostringstream
        std::unique_ptr<std::ostringstream> message;
    };
    std::vector<DeferredExpectation> mDeferredExpectations;

    // Assuming the data is mapped, checks all expectations
    void ResolveExpectations();

    dawn_native::Adapter mBackendAdapter;

    std::unique_ptr<dawn_platform::Platform> mTestPlatform;
};

// Skip a test when the given condition is satisfied.
#define DAWN_SKIP_TEST_IF(condition)                            \
    do {                                                        \
        if (condition) {                                        \
            dawn::InfoLog() << "Test skipped: " #condition "."; \
            GTEST_SKIP();                                       \
            return;                                             \
        }                                                       \
    } while (0)

#define EXPECT_DEPRECATION_WARNING(statement)                                    \
    do {                                                                         \
        if (UsesWire()) {                                                        \
            statement;                                                           \
        } else {                                                                 \
            size_t warningsBefore =                                              \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                           \
            size_t warningsAfter =                                               \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                        \
            if (!HasToggleEnabled("skip_validation")) {                          \
                EXPECT_EQ(warningsAfter, warningsBefore + 1);                    \
            }                                                                    \
            mLastWarningCount = warningsAfter;                                   \
        }                                                                        \
    } while (0)

template <typename Params = AdapterTestParam>
class DawnTestWithParams : public DawnTestBase, public ::testing::TestWithParam<Params> {
  protected:
    DawnTestWithParams();
    ~DawnTestWithParams() override = default;

    void SetUp() override {
        DawnTestBase::SetUp();
    }

    void TearDown() override {
        DawnTestBase::TearDown();
    }
};

template <typename Params>
DawnTestWithParams<Params>::DawnTestWithParams() : DawnTestBase(this->GetParam()) {
}

using DawnTest = DawnTestWithParams<>;

// Helpers to get the first element of a __VA_ARGS__ without triggering empty __VA_ARGS__ warnings.
#define DAWN_INTERNAL_PP_GET_HEAD(firstParam, ...) firstParam
#define DAWN_PP_GET_HEAD(...) DAWN_INTERNAL_PP_GET_HEAD(__VA_ARGS__, dummyArg)

// Instantiate the test once for each backend provided after the first argument. Use it like this:
//     DAWN_INSTANTIATE_TEST(MyTestFixture, MetalBackend, OpenGLBackend)
#define DAWN_INSTANTIATE_TEST(testName, ...)                                            \
    const decltype(DAWN_PP_GET_HEAD(__VA_ARGS__)) testName##params[] = {__VA_ARGS__};   \
    INSTANTIATE_TEST_SUITE_P(                                                           \
        , testName,                                                                     \
        testing::ValuesIn(::detail::GetAvailableAdapterTestParamsForBackends(           \
            testName##params, sizeof(testName##params) / sizeof(testName##params[0]))), \
        testing::PrintToStringParamName());                                             \
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(testName)

namespace detail {
    // Helper functions used for DAWN_INSTANTIATE_TEST
    bool IsBackendAvailable(wgpu::BackendType type);
    std::vector<AdapterTestParam> GetAvailableAdapterTestParamsForBackends(
        const BackendTestConfig* params,
        size_t numParams);

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
    extern template class ExpectEq<int16_t>;
    extern template class ExpectEq<uint32_t>;
    extern template class ExpectEq<uint64_t>;
    extern template class ExpectEq<RGBA8>;
    extern template class ExpectEq<float>;

    template <typename T>
    class ExpectBetweenColors : public Expectation {
      public:
        // Inclusive for now
        ExpectBetweenColors(T value0, T value1);
        testing::AssertionResult Check(const void* data, size_t size) override;

      private:
        std::vector<T> mLowerColorChannels;
        std::vector<T> mHigherColorChannels;

        // used for printing error
        std::vector<T> mValues0;
        std::vector<T> mValues1;
    };
    // A color is considered between color0 and color1 when all channel values are within range of
    // each counterparts. It doesn't matter which value is higher or lower. Essentially color =
    // lerp(color0, color1, t) where t is [0,1]. But I don't want to be too strict here.
    extern template class ExpectBetweenColors<RGBA8>;
}  // namespace detail

#endif  // TESTS_DAWNTEST_H_
