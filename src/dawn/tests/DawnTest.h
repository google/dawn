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

#ifndef SRC_DAWN_TESTS_DAWNTEST_H_
#define SRC_DAWN_TESTS_DAWNTEST_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/Preprocessor.h"
#include "dawn/dawn_proc_table.h"
#include "dawn/native/DawnNative.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/ParamGenerator.h"
#include "dawn/tests/ToggleParser.h"
#include "dawn/utils/ScopedAutoreleasePool.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/webgpu_cpp.h"
#include "dawn/webgpu_cpp_print.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Getting data back from Dawn is done in an async manners so all expectations are "deferred"
// until the end of the test. Also expectations use a copy to a MapRead buffer to get the data
// so resources should have the CopySrc allowed usage bit if you want to add expectations on
// them.

// AddBufferExpectation is defined in DawnTestBase as protected function. This ensures the macro can
// only be used in derivd class of DawnTestBase. Use "this" pointer to ensure the macro works with
// CRTP.
#define EXPECT_BUFFER(buffer, offset, size, expectation) \
    this->AddBufferExpectation(__FILE__, __LINE__, buffer, offset, size, expectation)

#define EXPECT_BUFFER_U8_EQ(expected, buffer, offset) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint8_t), new ::detail::ExpectEq<uint8_t>(expected))

#define EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, offset, count) \
    EXPECT_BUFFER(buffer, offset, sizeof(uint8_t) * (count),       \
                  new ::detail::ExpectEq<uint8_t>(expected, count))

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
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, {x, y})

#define EXPECT_PIXEL_FLOAT_EQ(expected, texture, x, y) \
    AddTextureExpectation(__FILE__, __LINE__, expected, texture, {x, y})

#define EXPECT_PIXEL_FLOAT16_EQ(expected, texture, x, y) \
    AddTextureExpectation<float, uint16_t>(__FILE__, __LINE__, expected, texture, {x, y})

#define EXPECT_PIXEL_RGBA8_BETWEEN(color0, color1, texture, x, y) \
    AddTextureBetweenColorsExpectation(__FILE__, __LINE__, color0, color1, texture, x, y)

#define EXPECT_TEXTURE_EQ(...) AddTextureExpectation(__FILE__, __LINE__, __VA_ARGS__)

#define EXPECT_TEXTURE_FLOAT16_EQ(...) \
    AddTextureExpectation<float, uint16_t>(__FILE__, __LINE__, __VA_ARGS__)

#define ASSERT_DEVICE_ERROR_MSG_ON(device, statement, matcher)                    \
    FlushWire();                                                                  \
    EXPECT_CALL(mDeviceErrorCallback,                                             \
                Call(testing::Ne(WGPUErrorType_NoError), matcher, device.Get())); \
    statement;                                                                    \
    FlushWire();                                                                  \
    testing::Mock::VerifyAndClearExpectations(&mDeviceErrorCallback);             \
    do {                                                                          \
    } while (0)

#define ASSERT_DEVICE_ERROR_MSG(statement, matcher) \
    ASSERT_DEVICE_ERROR_MSG_ON(this->device, statement, matcher)

#define ASSERT_DEVICE_ERROR_ON(device, statement) \
    ASSERT_DEVICE_ERROR_MSG_ON(device, statement, testing::_)

#define ASSERT_DEVICE_ERROR(statement) ASSERT_DEVICE_ERROR_MSG(statement, testing::_)

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
    class CustomTextureExpectation;

    template <typename T, typename U = T>
    class ExpectEq;
    template <typename T>
    class ExpectBetweenColors;
}  // namespace detail

namespace dawn::wire {
    class CommandHandler;
    class WireClient;
    class WireServer;
}  // namespace dawn::wire

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
    dawn::native::BackendValidationLevel GetBackendValidationLevel() const;
    dawn::native::Instance* GetInstance() const;
    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;
    bool HasBackendTypeFilter() const;
    wgpu::BackendType GetBackendTypeFilter() const;
    const char* GetWireTraceDir() const;
    GLFWwindow* GetOpenGLWindow() const;
    GLFWwindow* GetOpenGLESWindow() const;

    const std::vector<std::string>& GetEnabledToggles() const;
    const std::vector<std::string>& GetDisabledToggles() const;

    bool RunSuppressedTests() const;

  protected:
    std::unique_ptr<dawn::native::Instance> mInstance;

  private:
    void ParseArgs(int argc, char** argv);
    std::unique_ptr<dawn::native::Instance> CreateInstanceAndDiscoverAdapters();
    void SelectPreferredAdapterProperties(const dawn::native::Instance* instance);
    void PrintTestConfigurationAndAdapterInfo(dawn::native::Instance* instance) const;

    bool mUseWire = false;
    dawn::native::BackendValidationLevel mBackendValidationLevel =
        dawn::native::BackendValidationLevel::Disabled;
    bool mBeginCaptureOnStartup = false;
    bool mHasVendorIdFilter = false;
    uint32_t mVendorIdFilter = 0;
    bool mHasBackendTypeFilter = false;
    wgpu::BackendType mBackendTypeFilter;
    std::string mWireTraceDir;
    bool mRunSuppressedTests = false;

    ToggleParser mToggleParser;

    std::vector<wgpu::AdapterType> mDevicePreferences;
    std::vector<TestAdapterProperties> mAdapterProperties;

    std::unique_ptr<utils::PlatformDebugLogger> mPlatformDebugLogger;
    GLFWwindow* mOpenGLWindow;
    GLFWwindow* mOpenGLESWindow;
};

class DawnTestBase {
    friend class DawnPerfTestBase;

  public:
    explicit DawnTestBase(const AdapterTestParam& param);
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
    bool IsMacOS(int32_t majorVersion = -1, int32_t minorVersion = -1) const;

    bool UsesWire() const;
    bool IsBackendValidationEnabled() const;
    bool RunSuppressedTests() const;

    bool IsDXC() const;

    bool IsAsan() const;

    bool HasToggleEnabled(const char* workaround) const;

    void DestroyDevice(wgpu::Device device = nullptr);
    void LoseDeviceForTesting(wgpu::Device device = nullptr);

    bool HasVendorIdFilter() const;
    uint32_t GetVendorIdFilter() const;

    bool HasBackendTypeFilter() const;
    wgpu::BackendType GetBackendTypeFilter() const;

    wgpu::Instance GetInstance() const;
    dawn::native::Adapter GetAdapter() const;

    virtual std::unique_ptr<dawn::platform::Platform> CreateTestPlatform();

    struct PrintToStringParamName {
        explicit PrintToStringParamName(const char* test);
        std::string SanitizeParamName(std::string paramName, size_t index) const;

        template <class ParamType>
        std::string operator()(const ::testing::TestParamInfo<ParamType>& info) const {
            return SanitizeParamName(::testing::PrintToStringParamName()(info), info.index);
        }

        std::string mTest;
    };

  protected:
    wgpu::Device device;
    wgpu::Queue queue;

    DawnProcTable backendProcs = {};
    WGPUDevice backendDevice = nullptr;

    size_t mLastWarningCount = 0;

    // Mock callbacks tracking errors and destruction. Device lost is a nice mock since tests that
    // do not care about device destruction can ignore the callback entirely.
    testing::MockCallback<WGPUErrorCallback> mDeviceErrorCallback;
    testing::NiceMock<testing::MockCallback<WGPUDeviceLostCallback>> mDeviceLostCallback;

    // Helper methods to implement the EXPECT_ macros
    std::ostringstream& AddBufferExpectation(const char* file,
                                             int line,
                                             const wgpu::Buffer& buffer,
                                             uint64_t offset,
                                             uint64_t size,
                                             detail::Expectation* expectation);

    // T - expected value Type
    // U - actual value Type (defaults = T)
    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              wgpu::TextureFormat format,
                                              T tolerance = 0,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        uint32_t texelBlockSize = utils::GetTexelBlockSizeInBytes(format);
        uint32_t texelComponentCount = utils::GetWGSLRenderableColorTextureComponentCount(format);

        return AddTextureExpectationImpl(
            file, line,
            new detail::ExpectEq<T, U>(
                expectedData,
                texelComponentCount * extent.width * extent.height * extent.depthOrArrayLayers,
                tolerance),
            texture, origin, extent, level, aspect, texelBlockSize, bytesPerRow);
    }

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T* expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(
            file, line,
            new detail::ExpectEq<T, U>(expectedData,
                                       extent.width * extent.height * extent.depthOrArrayLayers),
            texture, origin, extent, level, aspect, sizeof(U), bytesPerRow);
    }

    template <typename T, typename U = T>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const T& expectedData,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(file, line, new detail::ExpectEq<T, U>(expectedData),
                                         texture, origin, {1, 1}, level, aspect, sizeof(U),
                                         bytesPerRow);
    }

    template <typename E,
              typename = typename std::enable_if<
                  std::is_base_of<detail::CustomTextureExpectation, E>::value>::type>
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              E* expectation,
                                              const wgpu::Texture& texture,
                                              wgpu::Origin3D origin,
                                              wgpu::Extent3D extent,
                                              uint32_t level = 0,
                                              wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                              uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(file, line, expectation, texture, origin, extent, level,
                                         aspect, expectation->DataSize(), bytesPerRow);
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
        wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
        uint32_t bytesPerRow = 0) {
        return AddTextureExpectationImpl(
            file, line, new detail::ExpectBetweenColors<T>(color0, color1), texture, {x, y}, {1, 1},
            level, aspect, sizeof(T), bytesPerRow);
    }

    std::ostringstream& ExpectSampledFloatData(wgpu::Texture texture,
                                               uint32_t width,
                                               uint32_t height,
                                               uint32_t componentCount,
                                               uint32_t arrayLayer,
                                               uint32_t mipLevel,
                                               detail::Expectation* expectation);

    std::ostringstream& ExpectMultisampledFloatData(wgpu::Texture texture,
                                                    uint32_t width,
                                                    uint32_t height,
                                                    uint32_t componentCount,
                                                    uint32_t sampleCount,
                                                    uint32_t arrayLayer,
                                                    uint32_t mipLevel,
                                                    detail::Expectation* expectation);

    std::ostringstream& ExpectSampledDepthData(wgpu::Texture depthTexture,
                                               uint32_t width,
                                               uint32_t height,
                                               uint32_t arrayLayer,
                                               uint32_t mipLevel,
                                               detail::Expectation* expectation);

    // Check depth by uploading expected data to a sampled texture, writing it out as a depth
    // attachment, and then using the "equals" depth test to check the contents are the same.
    // Check stencil by rendering a full screen quad and using the "equals" stencil test with
    // a stencil reference value. Note that checking stencil checks that the entire stencil
    // buffer is equal to the expected stencil value.
    std::ostringstream& ExpectAttachmentDepthStencilTestData(wgpu::Texture texture,
                                                             wgpu::TextureFormat format,
                                                             uint32_t width,
                                                             uint32_t height,
                                                             uint32_t arrayLayer,
                                                             uint32_t mipLevel,
                                                             std::vector<float> expectedDepth,
                                                             uint8_t* expectedStencil);

    std::ostringstream& ExpectAttachmentDepthTestData(wgpu::Texture texture,
                                                      wgpu::TextureFormat format,
                                                      uint32_t width,
                                                      uint32_t height,
                                                      uint32_t arrayLayer,
                                                      uint32_t mipLevel,
                                                      std::vector<float> expectedDepth) {
        return ExpectAttachmentDepthStencilTestData(texture, format, width, height, arrayLayer,
                                                    mipLevel, std::move(expectedDepth), nullptr);
    }

    std::ostringstream& ExpectAttachmentStencilTestData(wgpu::Texture texture,
                                                        wgpu::TextureFormat format,
                                                        uint32_t width,
                                                        uint32_t height,
                                                        uint32_t arrayLayer,
                                                        uint32_t mipLevel,
                                                        uint8_t expectedStencil) {
        return ExpectAttachmentDepthStencilTestData(texture, format, width, height, arrayLayer,
                                                    mipLevel, {}, &expectedStencil);
    }

    void WaitABit();
    void FlushWire();
    void WaitForAllOperations();

    bool SupportsFeatures(const std::vector<wgpu::FeatureName>& features);

    // Exposed device creation helper for tests to use when needing more than 1 device.
    wgpu::Device CreateDevice(std::string isolationKey = "");

    // Called in SetUp() to get the features required to be enabled in the tests. The tests must
    // check if the required features are supported by the adapter in this function and guarantee
    // the returned features are all supported by the adapter. The tests may provide different
    // code path to handle the situation when not all features are supported.
    virtual std::vector<wgpu::FeatureName> GetRequiredFeatures();

    virtual wgpu::RequiredLimits GetRequiredLimits(const wgpu::SupportedLimits&);

    const wgpu::AdapterProperties& GetAdapterProperties() const;

    // TODO(crbug.com/dawn/689): Use limits returned from the wire
    // This is implemented here because tests need to always query
    // the |backendDevice| since limits are not implemented in the wire.
    wgpu::SupportedLimits GetSupportedLimits();

  private:
    utils::ScopedAutoreleasePool mObjCAutoreleasePool;
    AdapterTestParam mParam;
    std::unique_ptr<utils::WireHelper> mWireHelper;

    // Internal device creation function for default device creation with some optional overrides.
    std::pair<wgpu::Device, WGPUDevice> CreateDeviceImpl(std::string isolationKey = "");

    std::ostringstream& AddTextureExpectationImpl(const char* file,
                                                  int line,
                                                  detail::Expectation* expectation,
                                                  const wgpu::Texture& texture,
                                                  wgpu::Origin3D origin,
                                                  wgpu::Extent3D extent,
                                                  uint32_t level,
                                                  wgpu::TextureAspect aspect,
                                                  uint32_t dataSize,
                                                  uint32_t bytesPerRow);

    std::ostringstream& ExpectSampledFloatDataImpl(wgpu::TextureView textureView,
                                                   const char* wgslTextureType,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   uint32_t componentCount,
                                                   uint32_t sampleCount,
                                                   detail::Expectation* expectation);

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
        uint32_t rowBytes = 0;
        uint32_t bytesPerRow = 0;
        std::unique_ptr<detail::Expectation> expectation;
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
        // Use unique_ptr because of missing move/copy constructors on std::basic_ostringstream
        std::unique_ptr<std::ostringstream> message;
    };
    std::vector<DeferredExpectation> mDeferredExpectations;

    // Assuming the data is mapped, checks all expectations
    void ResolveExpectations();

    dawn::native::Adapter mBackendAdapter;

    std::unique_ptr<dawn::platform::Platform> mTestPlatform;
};

#define DAWN_SKIP_TEST_IF_BASE(condition, type, reason)   \
    do {                                                  \
        if (condition) {                                  \
            dawn::InfoLog() << "Test " type ": " #reason; \
            GTEST_SKIP();                                 \
            return;                                       \
        }                                                 \
    } while (0)

// Skip a test which requires a feature or a toggle to be present / not present or some WIP
// features.
#define DAWN_TEST_UNSUPPORTED_IF(condition) \
    DAWN_SKIP_TEST_IF_BASE(condition, "unsupported", condition)

// Skip a test when the test failing on a specific HW / backend / OS combination. We can disable
// this macro with the command line parameter "--run-suppressed-tests".
#define DAWN_SUPPRESS_TEST_IF(condition) \
    DAWN_SKIP_TEST_IF_BASE(!RunSuppressedTests() && condition, "suppressed", condition)

#define EXPECT_DEPRECATION_WARNINGS(statement, n)                                 \
    do {                                                                          \
        if (UsesWire()) {                                                         \
            statement;                                                            \
        } else {                                                                  \
            size_t warningsBefore =                                               \
                dawn::native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                            \
            size_t warningsAfter =                                                \
                dawn::native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                         \
            if (!HasToggleEnabled("skip_validation")) {                           \
                EXPECT_EQ(warningsAfter, warningsBefore + n);                     \
            }                                                                     \
            mLastWarningCount = warningsAfter;                                    \
        }                                                                         \
    } while (0)
#define EXPECT_DEPRECATION_WARNING(statement) EXPECT_DEPRECATION_WARNINGS(statement, 1)

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

// Instantiate the test once for each backend provided after the first argument. Use it like this:
//     DAWN_INSTANTIATE_TEST(MyTestFixture, MetalBackend, OpenGLBackend)
#define DAWN_INSTANTIATE_TEST(testName, ...)                                            \
    const decltype(DAWN_PP_GET_HEAD(__VA_ARGS__)) testName##params[] = {__VA_ARGS__};   \
    INSTANTIATE_TEST_SUITE_P(                                                           \
        , testName,                                                                     \
        testing::ValuesIn(::detail::GetAvailableAdapterTestParamsForBackends(           \
            testName##params, sizeof(testName##params) / sizeof(testName##params[0]))), \
        DawnTestBase::PrintToStringParamName(#testName));                               \
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(testName)

// Instantiate the test once for each backend provided in the first param list.
// The test will be parameterized over the following param lists.
// Use it like this:
//     DAWN_INSTANTIATE_TEST_P(MyTestFixture, {MetalBackend(), OpenGLBackend()}, {A, B}, {1, 2})
// MyTestFixture must extend DawnTestWithParams<Param> where Param is a struct that extends
// AdapterTestParam, and whose constructor looks like:
//     Param(AdapterTestParam, ABorC, 12or3, ..., otherParams... )
//     You must also teach GTest how to print this struct.
//     https://github.com/google/googletest/blob/main/docs/advanced.md#teaching-googletest-how-to-print-your-values
// Macro DAWN_TEST_PARAM_STRUCT can help generate this struct.
#define DAWN_INSTANTIATE_TEST_P(testName, ...)                                                 \
    INSTANTIATE_TEST_SUITE_P(                                                                  \
        , testName, ::testing::ValuesIn(MakeParamGenerator<testName::ParamType>(__VA_ARGS__)), \
        DawnTestBase::PrintToStringParamName(#testName));                                      \
    GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(testName)

// Implementation for DAWN_TEST_PARAM_STRUCT to declare/print struct fields.
#define DAWN_TEST_PARAM_STRUCT_DECL_STRUCT_FIELD(Type) Type DAWN_PP_CONCATENATE(m, Type);
#define DAWN_TEST_PARAM_STRUCT_PRINT_STRUCT_FIELD(Type) \
    o << "; " << #Type << "=" << param.DAWN_PP_CONCATENATE(m, Type);

// Usage: DAWN_TEST_PARAM_STRUCT(Foo, TypeA, TypeB, ...)
// Generate a test param struct called Foo which extends AdapterTestParam and generated
// struct _Dawn_Foo. _Dawn_Foo has members of types TypeA, TypeB, etc. which are named mTypeA,
// mTypeB, etc. in the order they are placed in the macro argument list. Struct Foo should be
// constructed with an AdapterTestParam as the first argument, followed by a list of values
// to initialize the base _Dawn_Foo struct.
// It is recommended to use alias declarations so that stringified types are more readable.
// Example:
//   using MyParam = unsigned int;
//   DAWN_TEST_PARAM_STRUCT(FooParams, MyParam);
#define DAWN_TEST_PARAM_STRUCT(StructName, ...)                                                    \
    struct DAWN_PP_CONCATENATE(_Dawn_, StructName) {                                               \
        DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH)(DAWN_TEST_PARAM_STRUCT_DECL_STRUCT_FIELD,  \
                                                        __VA_ARGS__))                              \
    };                                                                                             \
    std::ostream& operator<<(std::ostream& o,                                                      \
                             const DAWN_PP_CONCATENATE(_Dawn_, StructName) & param) {              \
        DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH)(DAWN_TEST_PARAM_STRUCT_PRINT_STRUCT_FIELD, \
                                                        __VA_ARGS__))                              \
        return o;                                                                                  \
    }                                                                                              \
    struct StructName : AdapterTestParam, DAWN_PP_CONCATENATE(_Dawn_, StructName) {                \
        template <typename... Args>                                                                \
        StructName(const AdapterTestParam& param, Args&&... args)                                  \
            : AdapterTestParam(param),                                                             \
              DAWN_PP_CONCATENATE(_Dawn_, StructName){std::forward<Args>(args)...} {               \
        }                                                                                          \
    };                                                                                             \
    std::ostream& operator<<(std::ostream& o, const StructName& param) {                           \
        o << static_cast<const AdapterTestParam&>(param);                                          \
        o << "; " << static_cast<const DAWN_PP_CONCATENATE(_Dawn_, StructName)&>(param);           \
        return o;                                                                                  \
    }                                                                                              \
    static_assert(true, "require semicolon")

namespace detail {
    // Helper functions used for DAWN_INSTANTIATE_TEST
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
    // T - expected value Type
    // U - actual value Type (defaults = T)
    // This is expanded for float16 mostly where T=float, U=uint16_t
    template <typename T, typename U>
    class ExpectEq : public Expectation {
      public:
        explicit ExpectEq(T singleValue, T tolerance = {});
        ExpectEq(const T* values, const unsigned int count, T tolerance = {});

        testing::AssertionResult Check(const void* data, size_t size) override;

      private:
        std::vector<T> mExpected;
        T mTolerance;
    };
    extern template class ExpectEq<uint8_t>;
    extern template class ExpectEq<int16_t>;
    extern template class ExpectEq<uint32_t>;
    extern template class ExpectEq<uint64_t>;
    extern template class ExpectEq<RGBA8>;
    extern template class ExpectEq<float>;
    extern template class ExpectEq<float, uint16_t>;

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

    class CustomTextureExpectation : public Expectation {
      public:
        virtual ~CustomTextureExpectation() = default;
        virtual uint32_t DataSize() = 0;
    };

}  // namespace detail

#endif  // SRC_DAWN_TESTS_DAWNTEST_H_
