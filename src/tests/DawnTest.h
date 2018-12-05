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

#include "dawn/dawncpp.h"
#include "dawn_native/DawnNative.h"

#include <gtest/gtest.h>
#include <memory>

// Getting data back from Dawn is done in an async manners so all expectations are "deferred"
// until the end of the test. Also expectations use a copy to a MapRead buffer to get the data
// so resources should have the TransferSrc allowed usage bit if you want to add expectations on
// them.
#define EXPECT_BUFFER_U32_EQ(expected, buffer, offset)                         \
    AddBufferExpectation(__FILE__, __LINE__, buffer, offset, sizeof(uint32_t), \
                         new detail::ExpectEq<uint32_t>(expected))

#define EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, offset, count)                    \
    AddBufferExpectation(__FILE__, __LINE__, buffer, offset, sizeof(uint32_t) * count, \
                         new detail::ExpectEq<uint32_t>(expected, count))

#define EXPECT_BUFFER_U8_EQ(expected, buffer, offset)                         \
    AddBufferExpectation(__FILE__, __LINE__, buffer, offset, sizeof(uint8_t), \
                         new detail::ExpectEq<uint8_t>(expected))

// Test a pixel of the mip level 0 of a 2D texture.
#define EXPECT_PIXEL_RGBA8_EQ(expected, texture, x, y)                                  \
    AddTextureExpectation(__FILE__, __LINE__, texture, x, y, 1, 1, 0, 0, sizeof(RGBA8), \
                          new detail::ExpectEq<RGBA8>(expected))

#define EXPECT_TEXTURE_RGBA8_EQ(expected, texture, x, y, width, height, level, slice)     \
    AddTextureExpectation(__FILE__, __LINE__, texture, x, y, width, height, level, slice, \
                          sizeof(RGBA8),                                                  \
                          new detail::ExpectEq<RGBA8>(expected, (width) * (height)))

struct RGBA8 {
    constexpr RGBA8() : RGBA8(0, 0, 0, 0) {
    }
    constexpr RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {
    }
    bool operator==(const RGBA8& other) const;
    bool operator!=(const RGBA8& other) const;

    uint8_t r, g, b, a;
};
std::ostream& operator<<(std::ostream& stream, const RGBA8& color);

// Backend types used in the DAWN_INSTANTIATE_TEST
enum BackendType {
    D3D12Backend,
    MetalBackend,
    OpenGLBackend,
    VulkanBackend,
    NumBackendTypes,
};
std::ostream& operator<<(std::ostream& stream, BackendType backend);

namespace utils {
    class BackendBinding;
    class TerribleCommandBuffer;
}

namespace detail {
    class Expectation;
}

namespace dawn_wire {
    class CommandHandler;
}  // namespace dawn_wire

class DawnTest : public ::testing::TestWithParam<BackendType> {
  public:
    DawnTest();
    ~DawnTest();

    void SetUp() override;
    void TearDown() override;

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

  protected:
    dawn::Device device;
    dawn::Queue queue;
    dawn::SwapChain swapchain;

    // Helper methods to implement the EXPECT_ macros
    std::ostringstream& AddBufferExpectation(const char* file,
                                             int line,
                                             const dawn::Buffer& buffer,
                                             uint32_t offset,
                                             uint32_t size,
                                             detail::Expectation* expectation);
    std::ostringstream& AddTextureExpectation(const char* file,
                                              int line,
                                              const dawn::Texture& texture,
                                              uint32_t x,
                                              uint32_t y,
                                              uint32_t width,
                                              uint32_t height,
                                              uint32_t level,
                                              uint32_t slice,
                                              uint32_t pixelSize,
                                              detail::Expectation* expectation);

    void WaitABit();

    void SwapBuffersForCapture();

  private:
    // Things used to set up testing through the Wire.
    std::unique_ptr<dawn_wire::CommandHandler> mWireServer;
    std::unique_ptr<dawn_wire::CommandHandler> mWireClient;
    std::unique_ptr<utils::TerribleCommandBuffer> mC2sBuf;
    std::unique_ptr<utils::TerribleCommandBuffer> mS2cBuf;
    void FlushWire();

    // MapRead buffers used to get data for the expectations
    struct ReadbackSlot {
        dawn::Buffer buffer;
        uint32_t bufferSize;
        const void* mappedData = nullptr;
    };
    std::vector<ReadbackSlot> mReadbackSlots;

    // Maps all the buffers and fill ReadbackSlot::mappedData
    void MapSlotsSynchronously();
    static void SlotMapReadCallback(dawnBufferMapAsyncStatus status,
                                    const void* data,
                                    dawnCallbackUserdata userdata);
    size_t mNumPendingMapOperations = 0;

    // Reserve space where the data for an expectation can be copied
    struct ReadbackReservation {
        dawn::Buffer buffer;
        size_t slot;
        uint32_t offset;
    };
    ReadbackReservation ReserveReadback(uint32_t readbackSize);

    struct DeferredExpectation {
        const char* file;
        int line;
        size_t readbackSlot;
        uint32_t readbackOffset;
        uint32_t size;
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

    std::unique_ptr<utils::BackendBinding> mBinding;

    dawn_native::PCIInfo mPCIInfo;
};

// Instantiate the test once for each backend provided after the first argument. Use it like this:
//     DAWN_INSTANTIATE_TEST(MyTestFixture, MetalBackend, OpenGLBackend)
#define DAWN_INSTANTIATE_TEST(testName, firstParam, ...)                                           \
    const decltype(firstParam) testName##params[] = {firstParam, ##__VA_ARGS__};                   \
    INSTANTIATE_TEST_CASE_P(, testName,                                                            \
                            testing::ValuesIn(::detail::FilterBackends(                            \
                                testName##params, sizeof(testName##params) / sizeof(firstParam))), \
                            testing::PrintToStringParamName());

// Skip a test when the given condition is satisfied.
#define DAWN_SKIP_TEST_IF(condition)                               \
    if (condition) {                                               \
        std::cout << "Test skipped: " #condition "." << std::endl; \
        return;                                                    \
    }

namespace detail {
    // Helper functions used for DAWN_INSTANTIATE_TEST
    bool IsBackendAvailable(BackendType type);
    std::vector<BackendType> FilterBackends(const BackendType* types, size_t numParams);

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
