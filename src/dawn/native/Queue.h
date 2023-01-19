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

#ifndef SRC_DAWN_NATIVE_QUEUE_H_
#define SRC_DAWN_NATIVE_QUEUE_H_

#include <memory>

#include "dawn/common/SerialMap.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/DawnNative.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

// For the commands with async callback like 'MapAsync' and 'OnSubmittedWorkDone', we track the
// execution serials of completion in the queue for them. This implements 'CallbackTask' so that the
// aysnc callback can be fired by 'CallbackTaskManager' in a unified way. This also caches the
// finished serial, as the callback needs to use it in the trace event.
struct TrackTaskCallback : CallbackTask {
    explicit TrackTaskCallback(dawn::platform::Platform* platform) : mPlatform(platform) {}
    void SetFinishedSerial(ExecutionSerial serial);
    ~TrackTaskCallback() override = default;

  protected:
    dawn::platform::Platform* mPlatform = nullptr;
    // The serial by which time the callback can be fired.
    ExecutionSerial mSerial = kMaxExecutionSerial;
};

class QueueBase : public ApiObjectBase {
  public:
    ~QueueBase() override;

    static QueueBase* MakeError(DeviceBase* device);

    ObjectType GetType() const override;

    // Dawn API
    void APISubmit(uint32_t commandCount, CommandBufferBase* const* commands);
    void APIOnSubmittedWorkDone(uint64_t signalValue,
                                WGPUQueueWorkDoneCallback callback,
                                void* userdata);
    void APIWriteBuffer(BufferBase* buffer, uint64_t bufferOffset, const void* data, size_t size);
    void APIWriteTexture(const ImageCopyTexture* destination,
                         const void* data,
                         size_t dataSize,
                         const TextureDataLayout* dataLayout,
                         const Extent3D* writeSize);
    void APICopyTextureForBrowser(const ImageCopyTexture* source,
                                  const ImageCopyTexture* destination,
                                  const Extent3D* copySize,
                                  const CopyTextureForBrowserOptions* options);
    void APICopyExternalTextureForBrowser(const ImageCopyExternalTexture* source,
                                          const ImageCopyTexture* destination,
                                          const Extent3D* copySize,
                                          const CopyTextureForBrowserOptions* options);

    MaybeError WriteBuffer(BufferBase* buffer,
                           uint64_t bufferOffset,
                           const void* data,
                           size_t size);
    void TrackTask(std::unique_ptr<TrackTaskCallback> task, ExecutionSerial serial);
    void TrackTaskAfterEventualFlush(std::unique_ptr<TrackTaskCallback> task);
    void Tick(ExecutionSerial finishedSerial);
    void HandleDeviceLoss();

  protected:
    QueueBase(DeviceBase* device, const QueueDescriptor* descriptor);
    QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag);
    void DestroyImpl() override;

  private:
    MaybeError WriteTextureInternal(const ImageCopyTexture* destination,
                                    const void* data,
                                    size_t dataSize,
                                    const TextureDataLayout& dataLayout,
                                    const Extent3D* writeSize);
    MaybeError CopyTextureForBrowserInternal(const ImageCopyTexture* source,
                                             const ImageCopyTexture* destination,
                                             const Extent3D* copySize,
                                             const CopyTextureForBrowserOptions* options);
    MaybeError CopyExternalTextureForBrowserInternal(const ImageCopyExternalTexture* source,
                                                     const ImageCopyTexture* destination,
                                                     const Extent3D* copySize,
                                                     const CopyTextureForBrowserOptions* options);

    virtual MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) = 0;
    virtual MaybeError WriteBufferImpl(BufferBase* buffer,
                                       uint64_t bufferOffset,
                                       const void* data,
                                       size_t size);
    virtual MaybeError WriteTextureImpl(const ImageCopyTexture& destination,
                                        const void* data,
                                        const TextureDataLayout& dataLayout,
                                        const Extent3D& writeSize);

    MaybeError ValidateSubmit(uint32_t commandCount, CommandBufferBase* const* commands) const;
    MaybeError ValidateOnSubmittedWorkDone(uint64_t signalValue,
                                           WGPUQueueWorkDoneStatus* status) const;
    MaybeError ValidateWriteTexture(const ImageCopyTexture* destination,
                                    size_t dataSize,
                                    const TextureDataLayout& dataLayout,
                                    const Extent3D* writeSize) const;

    void SubmitInternal(uint32_t commandCount, CommandBufferBase* const* commands);

    SerialMap<ExecutionSerial, std::unique_ptr<TrackTaskCallback>> mTasksInFlight;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_QUEUE_H_
