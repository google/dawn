// Copyright 2017 The NXT Authors
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

#include "tests/NXTTest.h"

#include "utils/BackendBinding.h"

#include "GLFW/glfw3.h"

#include <cassert>
#define ASSERT assert

namespace {

    utils::BackendType ParamToBackendType(BackendType type) {
        switch(type) {
            case D3D12Backend:
                return utils::BackendType::D3D12;
            case MetalBackend:
                return utils::BackendType::Metal;
            case OpenGLBackend:
                return utils::BackendType::OpenGL;
            case VulkanBackend:
                return utils::BackendType::Vulkan;
            default:
                ASSERT(false);
                return utils::BackendType::Null;
        }
    }

    std::string ParamName(BackendType type) {
        switch(type) {
            case D3D12Backend:
                return "D3D12";
            case MetalBackend:
                return "Metal";
            case OpenGLBackend:
                return "OpenGL";
            case VulkanBackend:
                return "Vulkan";
            default:
                ASSERT(false);
                return "";
        }
    }

    // Windows don't usually like to be bound to one API than the other, for example switching
    // from Vulkan to OpenGL causes crashes on some drivers. Because of this, we lazily created
    // a window for each backing API.
    GLFWwindow* windows[NumBackendTypes];

    // Creates a GLFW window set up for use with a given backend.
    GLFWwindow* GetWindowForBackend(utils::BackendBinding* binding, BackendType type) {
        GLFWwindow** window = &windows[type];

        if (*window != nullptr) {
            return *window;
        }

        if (!glfwInit()) {
            return nullptr;
        }

        glfwDefaultWindowHints();
        binding->SetupGLFWWindowHints();

        std::string windowName = "NXT " + ParamName(type) + " test window";
        *window = glfwCreateWindow(400, 400, windowName.c_str(), nullptr, nullptr);

        return *window;
    }

    // End2end tests should test valid commands produce the expected result so no error
    // should happen. Failure cases should be tested in the validation tests.
    void DeviceErrorCauseTestFailure(const char* message, nxtCallbackUserdata) {
         FAIL() << "Device level failure: " << message;
    }

    struct MapReadUserdata {
        NXTTest* test;
        size_t slot;
    };
}

NXTTest::~NXTTest() {
    // We need to destroy child objects before the Device
    readbackSlots.clear();
    queue = nxt::Queue();
    device = nxt::Device();

    delete binding;
    binding = nullptr;

    nxtSetProcs(nullptr);
}

void NXTTest::SetUp() {
    binding = utils::CreateBinding(ParamToBackendType(GetParam()));
    ASSERT(binding != nullptr);

    GLFWwindow* testWindow = GetWindowForBackend(binding, GetParam());
    ASSERT(testWindow != nullptr);

    binding->SetWindow(testWindow);

    nxtDevice backendDevice;
    nxtProcTable backendProcs;
    binding->GetProcAndDevice(&backendProcs, &backendDevice);

    nxtSetProcs(&backendProcs);
    device = nxt::Device::Acquire(backendDevice);
    queue = device.CreateQueueBuilder().GetResult();

    device.SetErrorCallback(DeviceErrorCauseTestFailure, 0);
}

void NXTTest::TearDown() {
    MapSlotsSynchronously();
    ResolveExpectations();

    for (auto& expectation : deferredExpectations) {
        delete expectation.expectation;
        expectation.expectation = nullptr;
    }
}

void NXTTest::AddBufferExpectation(const char* file, int line, const nxt::Buffer& buffer, uint32_t offset, uint32_t size, detail::Expectation* expectation) {
    nxt::Buffer source = buffer.Clone();

    auto readback = ReserveReadback(size);

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the buffer might have been modified.
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(source, nxt::BufferUsageBit::TransferSrc)
        .TransitionBufferUsage(readback.buffer, nxt::BufferUsageBit::TransferDst)
        .CopyBufferToBuffer(source, offset, readback.buffer, readback.offset, size)
        .GetResult();

    queue.Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.expectation = expectation;

    deferredExpectations.push_back(deferred);
}

void NXTTest::AddTextureExpectation(const char* file, int line, const nxt::Texture& texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t pixelSize, detail::Expectation* expectation) {
    nxt::Texture source = texture.Clone();
    uint32_t size = width * height * pixelSize;

    auto readback = ReserveReadback(size);

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the texture might have been modified.
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionTextureUsage(source, nxt::TextureUsageBit::TransferSrc)
        .TransitionBufferUsage(readback.buffer, nxt::BufferUsageBit::TransferDst)
        .CopyTextureToBuffer(source, x, y, 0, width, height, 1, 0, readback.buffer, readback.offset)
        .GetResult();

    queue.Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.expectation = expectation;

    deferredExpectations.push_back(deferred);
}


NXTTest::ReadbackReservation NXTTest::ReserveReadback(uint32_t readbackSize) {
    // For now create a new MapRead buffer for each readback
    // TODO(cwallez@chromium.org): eventually make bigger buffers and allocate linearly?
    ReadbackSlot slot;
    slot.bufferSize = readbackSize;
    slot.buffer = device.CreateBufferBuilder()
        .SetSize(readbackSize)
        .SetAllowedUsage(nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();

    ReadbackReservation reservation;
    reservation.buffer = slot.buffer.Clone();
    reservation.slot = readbackSlots.size();
    reservation.offset = 0;

    readbackSlots.push_back(std::move(slot));
    return reservation;
}

void NXTTest::MapSlotsSynchronously() {
    // Initialize numPendingMapOperations before mapping, just in case the callback is called immediately.
    numPendingMapOperations = readbackSlots.size();

    // Map all readback slots
    for (size_t i = 0; i < readbackSlots.size(); ++i) {
        auto userdata = new MapReadUserdata{this, i};

        auto& slot = readbackSlots[i];
        slot.buffer.TransitionUsage(nxt::BufferUsageBit::MapRead);
        slot.buffer.MapReadAsync(0, slot.bufferSize, SlotMapReadCallback, static_cast<nxt::CallbackUserdata>(reinterpret_cast<uintptr_t>(userdata)));
    }

    // Busy wait until all map operations are done.
    // TODO(cwallez@chromium.org): usleep a bit?
    while (numPendingMapOperations != 0) {
        device.Tick();
    }
}

// static
void NXTTest::SlotMapReadCallback(nxtBufferMapReadStatus status, const void* data, nxtCallbackUserdata userdata_) {
    ASSERT(status == NXT_BUFFER_MAP_READ_STATUS_SUCCESS);

    auto userdata = reinterpret_cast<MapReadUserdata*>(static_cast<uintptr_t>(userdata_));
    userdata->test->readbackSlots[userdata->slot].mappedData = data;
    userdata->test->numPendingMapOperations --;

    delete userdata;
}

void NXTTest::ResolveExpectations() {
    for(const auto& expectation : deferredExpectations) {
        ASSERT(readbackSlots[expectation.readbackSlot].mappedData != nullptr);

        // Get a pointer to the mapped copy of the data for the expectation.
        const char* data = reinterpret_cast<const char*>(readbackSlots[expectation.readbackSlot].mappedData);
        data += expectation.readbackOffset;

        // Get the result for the expectation and add context to failures
        testing::AssertionResult result = expectation.expectation->Check(data, expectation.size);
        if (!result) {
            result << " Expectation created at " << expectation.file << ":" << expectation.line;
        }

        EXPECT_TRUE(result);
    }
}

bool RGBA8::operator==(const RGBA8& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

bool RGBA8::operator!=(const RGBA8& other) const {
    return !(*this == other);
}

std::ostream& operator<< (std::ostream& stream, const RGBA8& color) {
    return stream << "RGBA8(" <<
        static_cast<int>(color.r) << ", " <<
        static_cast<int>(color.g) << ", " <<
        static_cast<int>(color.b) << ", " <<
        static_cast<int>(color.a) << ")";
}

namespace detail {
    bool IsBackendAvailable(BackendType type) {
        #if defined(__APPLE__)
            return type == MetalBackend;
        #elif defined(_WIN32)
            return type == D3D12Backend;
        #elif __linux__
            // Temporarily silence a warning while Linux doesn't have a backend that can be tested.
            (void) type;
            return false;
        #else
            return false;
        #endif
    }

    std::vector<BackendType> FilterBackends(const BackendType* types, size_t numParams) {
        std::vector<BackendType> backends;

        for (size_t i = 0; i < numParams; ++i) {
            if (IsBackendAvailable(types[i])) {
                backends.push_back(types[i]);
            }
        }
        return backends;
    }

    // Helper classes to set expectations

    template<typename T>
    ExpectEq<T>::ExpectEq(T singleValue) {
        expected.push_back(singleValue);
    }

    template<typename T>
    ExpectEq<T>::ExpectEq(const T* values, const unsigned int count) {
        expected.assign(values, values + count);
    }

    template<typename T>
    testing::AssertionResult ExpectEq<T>::Check(const void* data, size_t size) {
        ASSERT(size == sizeof(T) * expected.size());

        const T* actual = reinterpret_cast<const T*>(data);
        for (size_t i = 0; i < expected.size(); ++i) {
            if (actual[i] != expected[i]) {
                return testing::AssertionFailure() << "Expected data[" << i << "] to be " << expected[i] << ", actual " << actual[i] << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }

    template class ExpectEq<uint32_t>;
    template class ExpectEq<RGBA8>;
}
