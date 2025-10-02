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

#ifndef SRC_DAWN_NATIVE_WEBGPU_CAPTURECONTEXT_H_
#define SRC_DAWN_NATIVE_WEBGPU_CAPTURECONTEXT_H_

#include <cstdint>
#include <ostream>
#include <string>

#include "absl/container/flat_hash_set.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/Serialization.h"

namespace dawn::native {

class CommandBufferBase;

}  // namespace dawn::native

namespace dawn::native::webgpu {

class CaptureContext;
class RecordableObject;

class CaptureContext {
  public:
    explicit CaptureContext(Device* device,
                            std::ostream& commandStream,
                            std::ostream& contentStream);
    ~CaptureContext();

    // Add resources both, creates an id for the resource AND captures its
    // description if it has not already been captured which is effectively
    // capturing an implicit call to createXXX.
    template <typename T>
    schema::ObjectId AddResource(T* object) {
        assert(object != nullptr);
        Ref<ApiObjectBase> ref(object);
        auto it = mObjectIds.find(ref);
        if (it != mObjectIds.end()) {
            return it->second;
        }

        const auto backendObject = ToBackend(object);
        backendObject->AddReferenced(*this);

        schema::ObjectId newId = mNextObjectId++;
        mObjectIds[ref] = newId;
        CaptureCreation(newId, object->GetLabel(), backendObject);

        return newId;
    }

    // You must have called AddResource at some point before calling GetId.
    template <typename T>
    schema::ObjectId GetId(T* object) {
        if (object == nullptr) {
            return 0;
        }

        auto it = mObjectIds.find(Ref<ApiObjectBase>(object));
        DAWN_ASSERT(it != mObjectIds.end());
        return it->second;
    }

    void WriteCommandBytes(const void* data, size_t size);

    void CaptureQueueWriteBuffer(BufferBase* buffer,
                                 uint64_t bufferOffset,
                                 const void* data,
                                 size_t size);

  private:
    WGPUBuffer GetCopyBuffer();
    void WriteContentBytes(const void* data, size_t size);
    void CaptureCreation(schema::ObjectId id,
                         const std::string& label,
                         const RecordableObject* object);

    // This is here for debugging. So that at debug time you can see what how many command bytes
    // have been written. and compare that to how many have been read when replaying.
    uint64_t mCommandBytesWritten = 0;

    Device* mDevice;
    std::ostream& mCommandStream;
    std::ostream& mContentStream;
    absl::flat_hash_map<Ref<ApiObjectBase>, schema::ObjectId> mObjectIds;
    schema::ObjectId mNextObjectId = 1;

    WGPUBuffer mCopyBuffer = nullptr;
};

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_CAPTURECONTEXT_H_
