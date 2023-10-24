// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/Queue.h"

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/FutureUtils.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/CopyTextureForBrowserHelper.h"
#include "dawn/native/Device.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/Texture.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "dawn/webgpu.h"

namespace dawn::native {

namespace {

void CopyTextureData(uint8_t* dstPointer,
                     const uint8_t* srcPointer,
                     uint32_t depth,
                     uint32_t rowsPerImage,
                     uint64_t imageAdditionalStride,
                     uint32_t actualBytesPerRow,
                     uint32_t dstBytesPerRow,
                     uint32_t srcBytesPerRow) {
    bool copyWholeLayer = actualBytesPerRow == dstBytesPerRow && dstBytesPerRow == srcBytesPerRow;
    bool copyWholeData = copyWholeLayer && imageAdditionalStride == 0;

    if (!copyWholeLayer) {  // copy row by row
        for (uint32_t d = 0; d < depth; ++d) {
            for (uint32_t h = 0; h < rowsPerImage; ++h) {
                memcpy(dstPointer, srcPointer, actualBytesPerRow);
                dstPointer += dstBytesPerRow;
                srcPointer += srcBytesPerRow;
            }
            srcPointer += imageAdditionalStride;
        }
    } else {
        uint64_t layerSize = uint64_t(rowsPerImage) * actualBytesPerRow;
        if (!copyWholeData) {  // copy layer by layer
            for (uint32_t d = 0; d < depth; ++d) {
                memcpy(dstPointer, srcPointer, layerSize);
                dstPointer += layerSize;
                srcPointer += layerSize + imageAdditionalStride;
            }
        } else {  // do a single copy
            memcpy(dstPointer, srcPointer, layerSize * depth);
        }
    }
}

ResultOrError<UploadHandle> UploadTextureDataAligningBytesPerRowAndOffset(
    DeviceBase* device,
    const void* data,
    uint32_t alignedBytesPerRow,
    uint32_t optimallyAlignedBytesPerRow,
    uint32_t alignedRowsPerImage,
    const TextureDataLayout& dataLayout,
    bool hasDepthOrStencil,
    const TexelBlockInfo& blockInfo,
    const Extent3D& writeSizePixel) {
    uint64_t newDataSizeBytes;
    DAWN_TRY_ASSIGN(newDataSizeBytes,
                    ComputeRequiredBytesInCopy(blockInfo, writeSizePixel,
                                               optimallyAlignedBytesPerRow, alignedRowsPerImage));

    uint64_t optimalOffsetAlignment = device->GetOptimalBufferToTextureCopyOffsetAlignment();
    DAWN_ASSERT(IsPowerOfTwo(optimalOffsetAlignment));
    DAWN_ASSERT(IsPowerOfTwo(blockInfo.byteSize));
    // We need the offset to be aligned to both optimalOffsetAlignment and blockByteSize,
    // since both of them are powers of two, we only need to align to the max value.
    uint64_t offsetAlignment = std::max(optimalOffsetAlignment, uint64_t(blockInfo.byteSize));

    // Buffer offset alignments must follow additional restrictions when we copy with depth stencil
    // formats.
    if (hasDepthOrStencil) {
        offsetAlignment =
            std::max(offsetAlignment, device->GetBufferCopyOffsetAlignmentForDepthStencil());
    }

    UploadHandle uploadHandle;
    DAWN_TRY_ASSIGN(uploadHandle,
                    device->GetDynamicUploader()->Allocate(
                        newDataSizeBytes, device->GetPendingCommandSerial(), offsetAlignment));
    DAWN_ASSERT(uploadHandle.mappedBuffer != nullptr);

    uint8_t* dstPointer = static_cast<uint8_t*>(uploadHandle.mappedBuffer);
    const uint8_t* srcPointer = static_cast<const uint8_t*>(data);
    srcPointer += dataLayout.offset;

    uint32_t dataRowsPerImage = dataLayout.rowsPerImage;
    if (dataRowsPerImage == 0) {
        dataRowsPerImage = writeSizePixel.height / blockInfo.height;
    }

    DAWN_ASSERT(dataRowsPerImage >= alignedRowsPerImage);
    uint64_t imageAdditionalStride =
        dataLayout.bytesPerRow * (dataRowsPerImage - alignedRowsPerImage);

    CopyTextureData(dstPointer, srcPointer, writeSizePixel.depthOrArrayLayers, alignedRowsPerImage,
                    imageAdditionalStride, alignedBytesPerRow, optimallyAlignedBytesPerRow,
                    dataLayout.bytesPerRow);

    return uploadHandle;
}

struct SubmittedWorkDone : TrackTaskCallback {
    SubmittedWorkDone(dawn::platform::Platform* platform,
                      WGPUQueueWorkDoneCallback callback,
                      void* userdata)
        : TrackTaskCallback(platform), mCallback(callback), mUserdata(userdata) {}
    ~SubmittedWorkDone() override = default;

  private:
    void FinishImpl() override {
        DAWN_ASSERT(mCallback != nullptr);
        DAWN_ASSERT(mSerial != kMaxExecutionSerial);
        TRACE_EVENT1(mPlatform, General, "Queue::SubmittedWorkDone::Finished", "serial",
                     uint64_t(mSerial));
        mCallback(WGPUQueueWorkDoneStatus_Success, mUserdata);
        mCallback = nullptr;
    }
    void HandleDeviceLossImpl() override {
        DAWN_ASSERT(mCallback != nullptr);
        mCallback(WGPUQueueWorkDoneStatus_DeviceLost, mUserdata);
        mCallback = nullptr;
    }
    void HandleShutDownImpl() override { HandleDeviceLossImpl(); }

    WGPUQueueWorkDoneCallback mCallback = nullptr;
    void* mUserdata;
};

class ErrorQueue : public QueueBase {
  public:
    explicit ErrorQueue(DeviceBase* device, const char* label)
        : QueueBase(device, ObjectBase::kError, label) {}

  private:
    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override {
        DAWN_UNREACHABLE();
    }
    bool HasPendingCommands() const override { DAWN_UNREACHABLE(); }
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override { DAWN_UNREACHABLE(); }
    void ForceEventualFlushOfCommands() override { DAWN_UNREACHABLE(); }
    MaybeError WaitForIdleForDestruction() override { DAWN_UNREACHABLE(); }
};

struct WorkDoneEvent final : public EventManager::TrackedEvent {
    std::optional<wgpu::QueueWorkDoneStatus> mEarlyStatus;
    WGPUQueueWorkDoneCallback mCallback;
    void* mUserdata;

    // Create an event backed by the given SystemEventReceiver.
    WorkDoneEvent(DeviceBase* device,
                  const QueueWorkDoneCallbackInfo& callbackInfo,
                  SystemEventReceiver&& receiver)
        : TrackedEvent(device, callbackInfo.mode, std::move(receiver)),
          mCallback(callbackInfo.callback),
          mUserdata(callbackInfo.userdata) {}

    // Create an event that's ready at creation (for errors, etc.)
    WorkDoneEvent(DeviceBase* device,
                  const QueueWorkDoneCallbackInfo& callbackInfo,
                  wgpu::QueueWorkDoneStatus earlyStatus)
        : TrackedEvent(device, callbackInfo.mode, SystemEventReceiver::CreateAlreadySignaled()),
          mEarlyStatus(earlyStatus),
          mCallback(callbackInfo.callback),
          mUserdata(callbackInfo.userdata) {
        CompleteIfSpontaneous();
    }

    ~WorkDoneEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

    // TODO(crbug.com/dawn/2062): When adding support for mixed sources, return false here when
    // the device has the mixed sources feature enabled, and so can expose the fence as an OS event.
    bool MustWaitUsingDevice() const override { return true; }

    void Complete(EventCompletionType completionType) override {
        // WorkDoneEvent has no error cases other than the mEarlyStatus ones.
        wgpu::QueueWorkDoneStatus status = wgpu::QueueWorkDoneStatus::Success;
        if (completionType == EventCompletionType::Shutdown) {
            status = wgpu::QueueWorkDoneStatus::Unknown;
        } else if (mEarlyStatus) {
            status = mEarlyStatus.value();
        }

        mCallback(ToAPI(status), mUserdata);
    }
};

}  // namespace

// TrackTaskCallback

void TrackTaskCallback::SetFinishedSerial(ExecutionSerial serial) {
    mSerial = serial;
}

// QueueBase

QueueBase::QueueBase(DeviceBase* device, const QueueDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label) {
    GetObjectTrackingList()->Track(this);
}

QueueBase::QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label)
    : ApiObjectBase(device, tag, label) {}

QueueBase::~QueueBase() {
    DAWN_ASSERT(mTasksInFlight->Empty());
}

void QueueBase::DestroyImpl() {}

// static
QueueBase* QueueBase::MakeError(DeviceBase* device, const char* label) {
    return new ErrorQueue(device, label);
}

ObjectType QueueBase::GetType() const {
    return ObjectType::Queue;
}

void QueueBase::APISubmit(uint32_t commandCount, CommandBufferBase* const* commands) {
    MaybeError result = SubmitInternal(commandCount, commands);

    // Destroy the command buffers even if SubmitInternal failed. (crbug.com/dawn/1863)
    for (uint32_t i = 0; i < commandCount; ++i) {
        commands[i]->Destroy();
    }

    DAWN_UNUSED(GetDevice()->ConsumedError(
        std::move(result), "calling %s.Submit(%s)", this,
        ityp::span<uint32_t, CommandBufferBase* const>(commands, commandCount)));
}

void QueueBase::APIOnSubmittedWorkDone(WGPUQueueWorkDoneCallback callback, void* userdata) {
    // The error status depends on the type of error so we let the validation function choose it
    wgpu::QueueWorkDoneStatus status;
    if (GetDevice()->ConsumedError(ValidateOnSubmittedWorkDone(&status))) {
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(
            [callback, status, userdata] { callback(ToAPI(status), userdata); });
        return;
    }

    std::unique_ptr<SubmittedWorkDone> task =
        std::make_unique<SubmittedWorkDone>(GetDevice()->GetPlatform(), callback, userdata);

    // Technically we only need to wait for previously submitted work but OnSubmittedWorkDone is
    // also used to make sure ALL queue work is finished in tests, so we also wait for pending
    // commands (this is non-observable outside of tests so it's ok to do deviate a bit from the
    // spec).
    TrackTaskAfterEventualFlush(std::move(task));

    TRACE_EVENT1(GetDevice()->GetPlatform(), General, "Queue::APIOnSubmittedWorkDone", "serial",
                 uint64_t(GetDevice()->GetPendingCommandSerial()));
}

Future QueueBase::APIOnSubmittedWorkDoneF(const QueueWorkDoneCallbackInfo& callbackInfo) {
    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_ASSERT(callbackInfo.nextInChain == nullptr);

    Ref<EventManager::TrackedEvent> event;

    wgpu::QueueWorkDoneStatus validationEarlyStatus;
    if (GetDevice()->ConsumedError(ValidateOnSubmittedWorkDone(&validationEarlyStatus))) {
        // TODO(crbug.com/dawn/2021): This is here to pretend that things succeed when the device is
        // lost. When the old OnSubmittedWorkDone is removed then we can update
        // ValidateOnSubmittedWorkDone to just return the correct thing here.
        if (validationEarlyStatus == wgpu::QueueWorkDoneStatus::DeviceLost) {
            validationEarlyStatus = wgpu::QueueWorkDoneStatus::Success;
        }

        // Note: if the callback is spontaneous, it'll get called in here.
        event = AcquireRef(new WorkDoneEvent(GetDevice(), callbackInfo, validationEarlyStatus));
    } else {
        event = AcquireRef(new WorkDoneEvent(GetDevice(), callbackInfo, InsertWorkDoneEvent()));
    }

    FutureID futureID =
        GetInstance()->GetEventManager()->TrackEvent(callbackInfo.mode, std::move(event));

    return {futureID};
}

SystemEventReceiver QueueBase::InsertWorkDoneEvent() {
    // TODO(crbug.com/dawn/2058): Implement this in all backends and remove this default impl
    DAWN_CHECK(false);
}

void QueueBase::TrackTask(std::unique_ptr<TrackTaskCallback> task, ExecutionSerial serial) {
    // If the task depends on a serial which is not submitted yet, force a flush.
    if (serial > GetLastSubmittedCommandSerial()) {
        ForceEventualFlushOfCommands();
    }

    DAWN_ASSERT(serial <= GetScheduledWorkDoneSerial());

    // If the serial indicated command has been completed, the task will be moved to callback task
    // manager.
    if (serial <= GetCompletedCommandSerial()) {
        task->SetFinishedSerial(GetCompletedCommandSerial());
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(std::move(task));
    } else {
        mTasksInFlight->Enqueue(std::move(task), serial);
    }
}

void QueueBase::TrackTaskAfterEventualFlush(std::unique_ptr<TrackTaskCallback> task) {
    ForceEventualFlushOfCommands();
    TrackTask(std::move(task), GetScheduledWorkDoneSerial());
}

void QueueBase::TrackPendingTask(std::unique_ptr<TrackTaskCallback> task) {
    mTasksInFlight->Enqueue(std::move(task), GetPendingCommandSerial());
}

void QueueBase::Tick(ExecutionSerial finishedSerial) {
    // If a user calls Queue::Submit inside a task, for example in a Buffer::MapAsync callback,
    // then the device will be ticked, which in turns ticks the queue, causing reentrance here.
    // To prevent the reentrant call from invalidating mTasksInFlight while in use by the first
    // call, we remove the tasks to finish from the queue, update mTasksInFlight, then run the
    // callbacks.
    TRACE_EVENT1(GetDevice()->GetPlatform(), General, "Queue::Tick", "finishedSerial",
                 uint64_t(finishedSerial));

    std::vector<std::unique_ptr<TrackTaskCallback>> tasks;
    mTasksInFlight.Use([&](auto tasksInFlight) {
        for (auto& task : tasksInFlight->IterateUpTo(finishedSerial)) {
            tasks.push_back(std::move(task));
        }
        tasksInFlight->ClearUpTo(finishedSerial);
    });
    // Tasks' serials have passed. Move them to the callback task manager. They
    // are ready to be called.
    for (auto& task : tasks) {
        task->SetFinishedSerial(finishedSerial);
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(std::move(task));
    }
}

void QueueBase::HandleDeviceLoss() {
    mTasksInFlight.Use([&](auto tasksInFlight) {
        for (auto& task : tasksInFlight->IterateAll()) {
            task->OnDeviceLoss();
            GetDevice()->GetCallbackTaskManager()->AddCallbackTask(std::move(task));
        }
        tasksInFlight->Clear();
    });
}

void QueueBase::APIWriteBuffer(BufferBase* buffer,
                               uint64_t bufferOffset,
                               const void* data,
                               size_t size) {
    DAWN_UNUSED(
        GetDevice()->ConsumedError(WriteBuffer(buffer, bufferOffset, data, size),
                                   "calling %s.WriteBuffer(%s, (%d bytes), data, (%d bytes))", this,
                                   buffer, bufferOffset, size));
}

MaybeError QueueBase::WriteBuffer(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(ValidateWriteBuffer(GetDevice(), buffer, bufferOffset, size));
    DAWN_TRY(buffer->ValidateCanUseOnQueueNow());
    return WriteBufferImpl(buffer, bufferOffset, data, size);
}

MaybeError QueueBase::WriteBufferImpl(BufferBase* buffer,
                                      uint64_t bufferOffset,
                                      const void* data,
                                      size_t size) {
    if (size == 0) {
        return {};
    }

    DeviceBase* device = GetDevice();

    UploadHandle uploadHandle;
    DAWN_TRY_ASSIGN(uploadHandle,
                    device->GetDynamicUploader()->Allocate(size, device->GetPendingCommandSerial(),
                                                           kCopyBufferToBufferOffsetAlignment));
    DAWN_ASSERT(uploadHandle.mappedBuffer != nullptr);

    memcpy(uploadHandle.mappedBuffer, data, size);

    return device->CopyFromStagingToBuffer(uploadHandle.stagingBuffer, uploadHandle.startOffset,
                                           buffer, bufferOffset, size);
}

void QueueBase::APIWriteTexture(const ImageCopyTexture* destination,
                                const void* data,
                                size_t dataSize,
                                const TextureDataLayout* dataLayout,
                                const Extent3D* writeSize) {
    DAWN_UNUSED(GetDevice()->ConsumedError(
        WriteTextureInternal(destination, data, dataSize, *dataLayout, writeSize),
        "calling %s.WriteTexture(%s, (%u bytes), %s, %s)", this, destination, dataSize, dataLayout,
        writeSize));
}

MaybeError QueueBase::WriteTextureInternal(const ImageCopyTexture* destination,
                                           const void* data,
                                           size_t dataSize,
                                           const TextureDataLayout& dataLayout,
                                           const Extent3D* writeSize) {
    DAWN_TRY(ValidateWriteTexture(destination, dataSize, dataLayout, writeSize));

    if (writeSize->width == 0 || writeSize->height == 0 || writeSize->depthOrArrayLayers == 0) {
        return {};
    }

    const TexelBlockInfo& blockInfo =
        destination->texture->GetFormat().GetAspectInfo(destination->aspect).block;
    TextureDataLayout layout = dataLayout;
    ApplyDefaultTextureDataLayoutOptions(&layout, blockInfo, *writeSize);
    return WriteTextureImpl(*destination, data, layout, *writeSize);
}

MaybeError QueueBase::WriteTextureImpl(const ImageCopyTexture& destination,
                                       const void* data,
                                       const TextureDataLayout& dataLayout,
                                       const Extent3D& writeSizePixel) {
    const Format& format = destination.texture->GetFormat();
    const TexelBlockInfo& blockInfo = format.GetAspectInfo(destination.aspect).block;

    // We are only copying the part of the data that will appear in the texture.
    // Note that validating texture copy range ensures that writeSizePixel->width and
    // writeSizePixel->height are multiples of blockWidth and blockHeight respectively.
    DAWN_ASSERT(writeSizePixel.width % blockInfo.width == 0);
    DAWN_ASSERT(writeSizePixel.height % blockInfo.height == 0);
    uint32_t alignedBytesPerRow = writeSizePixel.width / blockInfo.width * blockInfo.byteSize;
    uint32_t alignedRowsPerImage = writeSizePixel.height / blockInfo.height;

    uint32_t optimalBytesPerRowAlignment = GetDevice()->GetOptimalBytesPerRowAlignment();
    uint32_t optimallyAlignedBytesPerRow = Align(alignedBytesPerRow, optimalBytesPerRowAlignment);

    UploadHandle uploadHandle;
    DAWN_TRY_ASSIGN(uploadHandle, UploadTextureDataAligningBytesPerRowAndOffset(
                                      GetDevice(), data, alignedBytesPerRow,
                                      optimallyAlignedBytesPerRow, alignedRowsPerImage, dataLayout,
                                      format.HasDepthOrStencil(), blockInfo, writeSizePixel));

    TextureDataLayout passDataLayout = dataLayout;
    passDataLayout.offset = uploadHandle.startOffset;
    passDataLayout.bytesPerRow = optimallyAlignedBytesPerRow;
    passDataLayout.rowsPerImage = alignedRowsPerImage;

    TextureCopy textureCopy;
    textureCopy.texture = destination.texture;
    textureCopy.mipLevel = destination.mipLevel;
    textureCopy.origin = destination.origin;
    textureCopy.aspect = ConvertAspect(format, destination.aspect);

    DeviceBase* device = GetDevice();

    return device->CopyFromStagingToTexture(uploadHandle.stagingBuffer, passDataLayout, textureCopy,
                                            writeSizePixel);
}

void QueueBase::APICopyTextureForBrowser(const ImageCopyTexture* source,
                                         const ImageCopyTexture* destination,
                                         const Extent3D* copySize,
                                         const CopyTextureForBrowserOptions* options) {
    DAWN_UNUSED(GetDevice()->ConsumedError(
        CopyTextureForBrowserInternal(source, destination, copySize, options)));
}

void QueueBase::APICopyExternalTextureForBrowser(const ImageCopyExternalTexture* source,
                                                 const ImageCopyTexture* destination,
                                                 const Extent3D* copySize,
                                                 const CopyTextureForBrowserOptions* options) {
    DAWN_UNUSED(GetDevice()->ConsumedError(
        CopyExternalTextureForBrowserInternal(source, destination, copySize, options)));
}

MaybeError QueueBase::CopyTextureForBrowserInternal(const ImageCopyTexture* source,
                                                    const ImageCopyTexture* destination,
                                                    const Extent3D* copySize,
                                                    const CopyTextureForBrowserOptions* options) {
    if (GetDevice()->IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(
            ValidateCopyTextureForBrowser(GetDevice(), source, destination, copySize, options),
            "validating CopyTextureForBrowser from %s to %s", source->texture,
            destination->texture);
    }

    return DoCopyTextureForBrowser(GetDevice(), source, destination, copySize, options);
}

MaybeError QueueBase::CopyExternalTextureForBrowserInternal(
    const ImageCopyExternalTexture* source,
    const ImageCopyTexture* destination,
    const Extent3D* copySize,
    const CopyTextureForBrowserOptions* options) {
    if (GetDevice()->IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateCopyExternalTextureForBrowser(GetDevice(), source, destination,
                                                               copySize, options),
                         "validating CopyExternalTextureForBrowser from %s to %s",
                         source->externalTexture, destination->texture);
    }

    return DoCopyExternalTextureForBrowser(GetDevice(), source, destination, copySize, options);
}

MaybeError QueueBase::ValidateSubmit(uint32_t commandCount,
                                     CommandBufferBase* const* commands) const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "Queue::ValidateSubmit");
    DAWN_TRY(GetDevice()->ValidateObject(this));

    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(GetDevice()->ValidateObject(commands[i]));
        DAWN_TRY(commands[i]->ValidateCanUseInSubmitNow());
        const CommandBufferResourceUsage& usages = commands[i]->GetResourceUsages();

        for (const BufferBase* buffer : usages.topLevelBuffers) {
            DAWN_TRY(buffer->ValidateCanUseOnQueueNow());
        }

        // Maybe track last usage for other resources, and use it to release resources earlier?
        for (const SyncScopeResourceUsage& scope : usages.renderPasses) {
            for (const BufferBase* buffer : scope.buffers) {
                DAWN_TRY(buffer->ValidateCanUseOnQueueNow());
            }

            for (const TextureBase* texture : scope.textures) {
                DAWN_TRY(texture->ValidateCanUseInSubmitNow());
            }

            for (const ExternalTextureBase* externalTexture : scope.externalTextures) {
                DAWN_TRY(externalTexture->ValidateCanUseInSubmitNow());
            }
        }

        for (const ComputePassResourceUsage& pass : usages.computePasses) {
            for (const BufferBase* buffer : pass.referencedBuffers) {
                DAWN_TRY(buffer->ValidateCanUseOnQueueNow());
            }
            for (const TextureBase* texture : pass.referencedTextures) {
                DAWN_TRY(texture->ValidateCanUseInSubmitNow());
            }
            for (const ExternalTextureBase* externalTexture : pass.referencedExternalTextures) {
                DAWN_TRY(externalTexture->ValidateCanUseInSubmitNow());
            }
        }

        for (const TextureBase* texture : usages.topLevelTextures) {
            DAWN_TRY(texture->ValidateCanUseInSubmitNow());
        }
        for (const QuerySetBase* querySet : usages.usedQuerySets) {
            DAWN_TRY(querySet->ValidateCanUseInSubmitNow());
        }
    }

    return {};
}

MaybeError QueueBase::ValidateOnSubmittedWorkDone(wgpu::QueueWorkDoneStatus* status) const {
    *status = wgpu::QueueWorkDoneStatus::DeviceLost;
    DAWN_TRY(GetDevice()->ValidateIsAlive());

    *status = wgpu::QueueWorkDoneStatus::Error;
    DAWN_TRY(GetDevice()->ValidateObject(this));

    return {};
}

MaybeError QueueBase::ValidateWriteTexture(const ImageCopyTexture* destination,
                                           size_t dataSize,
                                           const TextureDataLayout& dataLayout,
                                           const Extent3D* writeSize) const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

    DAWN_TRY(ValidateImageCopyTexture(GetDevice(), *destination, *writeSize));

    DAWN_INVALID_IF(dataLayout.offset > dataSize,
                    "Data offset (%u) is greater than the data size (%u).", dataLayout.offset,
                    dataSize);

    DAWN_INVALID_IF(!(destination->texture->GetUsage() & wgpu::TextureUsage::CopyDst),
                    "Usage (%s) of %s does not include %s.", destination->texture->GetUsage(),
                    destination->texture, wgpu::TextureUsage::CopyDst);

    DAWN_INVALID_IF(destination->texture->GetSampleCount() > 1, "Sample count (%u) of %s is not 1",
                    destination->texture->GetSampleCount(), destination->texture);

    DAWN_TRY(ValidateLinearToDepthStencilCopyRestrictions(*destination));
    // We validate texture copy range before validating linear texture data,
    // because in the latter we divide copyExtent.width by blockWidth and
    // copyExtent.height by blockHeight while the divisibility conditions are
    // checked in validating texture copy range.
    DAWN_TRY(ValidateTextureCopyRange(GetDevice(), *destination, *writeSize));

    const TexelBlockInfo& blockInfo =
        destination->texture->GetFormat().GetAspectInfo(destination->aspect).block;

    DAWN_TRY(ValidateLinearTextureData(dataLayout, dataSize, blockInfo, *writeSize));

    DAWN_TRY(destination->texture->ValidateCanUseInSubmitNow());

    return {};
}

MaybeError QueueBase::SubmitInternal(uint32_t commandCount, CommandBufferBase* const* commands) {
    DeviceBase* device = GetDevice();

    // If device is lost, don't let any commands be submitted
    DAWN_TRY(device->ValidateIsAlive());

    TRACE_EVENT0(device->GetPlatform(), General, "Queue::Submit");
    if (device->IsValidationEnabled()) {
        DAWN_TRY(ValidateSubmit(commandCount, commands));
    }
    DAWN_ASSERT(!IsError());

    DAWN_TRY(SubmitImpl(commandCount, commands));

    // Call Tick() to flush pending work.
    DAWN_TRY(device->Tick());

    return {};
}

}  // namespace dawn::native
