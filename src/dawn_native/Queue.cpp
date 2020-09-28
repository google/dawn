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

#include "dawn_native/Queue.h"

#include "common/Constants.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/ErrorScope.h"
#include "dawn_native/ErrorScopeTracker.h"
#include "dawn_native/Fence.h"
#include "dawn_native/FenceSignalTracker.h"
#include "dawn_native/QuerySet.h"
#include "dawn_native/Texture.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

#include <cstring>

namespace dawn_native {
    namespace {
        void CopyTextureData(uint8_t* dstPointer,
                             const uint8_t* srcPointer,
                             uint32_t depth,
                             uint32_t rowsPerImageInBlock,
                             uint64_t imageAdditionalStride,
                             uint32_t actualBytesPerRow,
                             uint32_t dstBytesPerRow,
                             uint32_t srcBytesPerRow) {
            bool copyWholeLayer =
                actualBytesPerRow == dstBytesPerRow && dstBytesPerRow == srcBytesPerRow;
            bool copyWholeData = copyWholeLayer && imageAdditionalStride == 0;

            if (!copyWholeLayer) {  // copy row by row
                for (uint32_t d = 0; d < depth; ++d) {
                    for (uint32_t h = 0; h < rowsPerImageInBlock; ++h) {
                        memcpy(dstPointer, srcPointer, actualBytesPerRow);
                        dstPointer += dstBytesPerRow;
                        srcPointer += srcBytesPerRow;
                    }
                    srcPointer += imageAdditionalStride;
                }
            } else {
                uint64_t layerSize = uint64_t(rowsPerImageInBlock) * actualBytesPerRow;
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
            const TexelBlockInfo& blockInfo,
            const Extent3D& writeSizePixel) {
            uint64_t newDataSizeBytes;
            DAWN_TRY_ASSIGN(
                newDataSizeBytes,
                ComputeRequiredBytesInCopy(blockInfo, writeSizePixel, optimallyAlignedBytesPerRow,
                                           alignedRowsPerImage));

            uint64_t optimalOffsetAlignment =
                device->GetOptimalBufferToTextureCopyOffsetAlignment();
            ASSERT(IsPowerOfTwo(optimalOffsetAlignment));
            ASSERT(IsPowerOfTwo(blockInfo.blockByteSize));
            // We need the offset to be aligned to both optimalOffsetAlignment and blockByteSize,
            // since both of them are powers of two, we only need to align to the max value.
            uint64_t offsetAlignment =
                std::max(optimalOffsetAlignment, uint64_t(blockInfo.blockByteSize));

            UploadHandle uploadHandle;
            DAWN_TRY_ASSIGN(uploadHandle, device->GetDynamicUploader()->Allocate(
                                              newDataSizeBytes, device->GetPendingCommandSerial(),
                                              offsetAlignment));
            ASSERT(uploadHandle.mappedBuffer != nullptr);

            uint8_t* dstPointer = static_cast<uint8_t*>(uploadHandle.mappedBuffer);
            const uint8_t* srcPointer = static_cast<const uint8_t*>(data);
            srcPointer += dataLayout.offset;

            uint32_t alignedRowsPerImageInBlock = alignedRowsPerImage / blockInfo.blockHeight;
            uint32_t dataRowsPerImageInBlock = dataLayout.rowsPerImage / blockInfo.blockHeight;
            if (dataRowsPerImageInBlock == 0) {
                dataRowsPerImageInBlock = writeSizePixel.height / blockInfo.blockHeight;
            }

            ASSERT(dataRowsPerImageInBlock >= alignedRowsPerImageInBlock);
            uint64_t imageAdditionalStride =
                dataLayout.bytesPerRow * (dataRowsPerImageInBlock - alignedRowsPerImageInBlock);

            CopyTextureData(dstPointer, srcPointer, writeSizePixel.depth,
                            alignedRowsPerImageInBlock, imageAdditionalStride, alignedBytesPerRow,
                            optimallyAlignedBytesPerRow, dataLayout.bytesPerRow);

            return uploadHandle;
        }

        class ErrorQueue : public QueueBase {
          public:
            ErrorQueue(DeviceBase* device) : QueueBase(device, ObjectBase::kError) {
            }

          private:
            MaybeError SubmitImpl(uint32_t commandCount,
                                  CommandBufferBase* const* commands) override {
                UNREACHABLE();
            }
        };

    }  // namespace

    // QueueBase

    QueueBase::QueueBase(DeviceBase* device) : ObjectBase(device) {
    }

    QueueBase::QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag) : ObjectBase(device, tag) {
    }

    // static
    QueueBase* QueueBase::MakeError(DeviceBase* device) {
        return new ErrorQueue(device);
    }

    void QueueBase::Submit(uint32_t commandCount, CommandBufferBase* const* commands) {
        SubmitInternal(commandCount, commands);

        for (uint32_t i = 0; i < commandCount; ++i) {
            commands[i]->Destroy();
        }
    }

    void QueueBase::Signal(Fence* fence, uint64_t apiSignalValue) {
        FenceAPISerial signalValue(apiSignalValue);

        DeviceBase* device = GetDevice();
        if (device->ConsumedError(ValidateSignal(fence, signalValue))) {
            return;
        }
        ASSERT(!IsError());

        fence->SetSignaledValue(signalValue);
        device->GetFenceSignalTracker()->UpdateFenceOnComplete(fence, signalValue);
        device->GetErrorScopeTracker()->TrackUntilLastSubmitComplete(
            device->GetCurrentErrorScope());
    }

    Fence* QueueBase::CreateFence(const FenceDescriptor* descriptor) {
        if (GetDevice()->ConsumedError(ValidateCreateFence(descriptor))) {
            return Fence::MakeError(GetDevice());
        }

        if (descriptor == nullptr) {
            FenceDescriptor defaultDescriptor = {};
            return new Fence(this, &defaultDescriptor);
        }
        return new Fence(this, descriptor);
    }

    void QueueBase::WriteBuffer(BufferBase* buffer,
                                uint64_t bufferOffset,
                                const void* data,
                                size_t size) {
        GetDevice()->ConsumedError(WriteBufferInternal(buffer, bufferOffset, data, size));
    }

    MaybeError QueueBase::WriteBufferInternal(BufferBase* buffer,
                                              uint64_t bufferOffset,
                                              const void* data,
                                              size_t size) {
        DAWN_TRY(ValidateWriteBuffer(buffer, bufferOffset, size));
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
        DAWN_TRY_ASSIGN(uploadHandle, device->GetDynamicUploader()->Allocate(
                                          size, device->GetPendingCommandSerial(),
                                          kCopyBufferToBufferOffsetAlignment));
        ASSERT(uploadHandle.mappedBuffer != nullptr);

        memcpy(uploadHandle.mappedBuffer, data, size);

        return device->CopyFromStagingToBuffer(uploadHandle.stagingBuffer, uploadHandle.startOffset,
                                               buffer, bufferOffset, size);
    }

    void QueueBase::WriteTexture(const TextureCopyView* destination,
                                 const void* data,
                                 size_t dataSize,
                                 const TextureDataLayout* dataLayout,
                                 const Extent3D* writeSize) {
        GetDevice()->ConsumedError(
            WriteTextureInternal(destination, data, dataSize, dataLayout, writeSize));
    }

    MaybeError QueueBase::WriteTextureInternal(const TextureCopyView* destination,
                                               const void* data,
                                               size_t dataSize,
                                               const TextureDataLayout* dataLayout,
                                               const Extent3D* writeSize) {
        DAWN_TRY(ValidateWriteTexture(destination, dataSize, dataLayout, writeSize));

        if (writeSize->width == 0 || writeSize->height == 0 || writeSize->depth == 0) {
            return {};
        }

        return WriteTextureImpl(*destination, data, *dataLayout, *writeSize);
    }

    MaybeError QueueBase::WriteTextureImpl(const TextureCopyView& destination,
                                           const void* data,
                                           const TextureDataLayout& dataLayout,
                                           const Extent3D& writeSizePixel) {
        const TexelBlockInfo& blockInfo =
            destination.texture->GetFormat().GetTexelBlockInfo(destination.aspect);

        // We are only copying the part of the data that will appear in the texture.
        // Note that validating texture copy range ensures that writeSizePixel->width and
        // writeSizePixel->height are multiples of blockWidth and blockHeight respectively.
        uint32_t alignedBytesPerRow =
            (writeSizePixel.width) / blockInfo.blockWidth * blockInfo.blockByteSize;
        uint32_t alignedRowsPerImage = writeSizePixel.height;

        uint32_t optimalBytesPerRowAlignment = GetDevice()->GetOptimalBytesPerRowAlignment();
        uint32_t optimallyAlignedBytesPerRow =
            Align(alignedBytesPerRow, optimalBytesPerRowAlignment);

        UploadHandle uploadHandle;
        DAWN_TRY_ASSIGN(uploadHandle,
                        UploadTextureDataAligningBytesPerRowAndOffset(
                            GetDevice(), data, alignedBytesPerRow, optimallyAlignedBytesPerRow,
                            alignedRowsPerImage, dataLayout, blockInfo, writeSizePixel));

        TextureDataLayout passDataLayout = dataLayout;
        passDataLayout.offset = uploadHandle.startOffset;
        passDataLayout.bytesPerRow = optimallyAlignedBytesPerRow;
        passDataLayout.rowsPerImage = alignedRowsPerImage;

        TextureCopy textureCopy;
        textureCopy.texture = destination.texture;
        textureCopy.mipLevel = destination.mipLevel;
        textureCopy.origin = destination.origin;
        textureCopy.aspect = ConvertAspect(destination.texture->GetFormat(), destination.aspect);

        return GetDevice()->CopyFromStagingToTexture(uploadHandle.stagingBuffer, passDataLayout,
                                                     &textureCopy, writeSizePixel);
    }
    MaybeError QueueBase::ValidateSubmit(uint32_t commandCount,
                                         CommandBufferBase* const* commands) const {
        TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "Queue::ValidateSubmit");
        DAWN_TRY(GetDevice()->ValidateObject(this));

        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(GetDevice()->ValidateObject(commands[i]));
            DAWN_TRY(commands[i]->ValidateCanUseInSubmitNow());

            const CommandBufferResourceUsage& usages = commands[i]->GetResourceUsages();

            for (const PassResourceUsage& passUsages : usages.perPass) {
                for (const BufferBase* buffer : passUsages.buffers) {
                    DAWN_TRY(buffer->ValidateCanUseOnQueueNow());
                }
                for (const TextureBase* texture : passUsages.textures) {
                    DAWN_TRY(texture->ValidateCanUseInSubmitNow());
                }
            }

            for (const BufferBase* buffer : usages.topLevelBuffers) {
                DAWN_TRY(buffer->ValidateCanUseOnQueueNow());
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

    MaybeError QueueBase::ValidateSignal(const Fence* fence, FenceAPISerial signalValue) const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(GetDevice()->ValidateObject(fence));

        if (fence->GetQueue() != this) {
            return DAWN_VALIDATION_ERROR(
                "Fence must be signaled on the queue on which it was created.");
        }
        if (signalValue <= fence->GetSignaledValue()) {
            return DAWN_VALIDATION_ERROR("Signal value less than or equal to fence signaled value");
        }
        return {};
    }

    MaybeError QueueBase::ValidateCreateFence(const FenceDescriptor* descriptor) const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        if (descriptor != nullptr) {
            DAWN_TRY(ValidateFenceDescriptor(descriptor));
        }

        return {};
    }

    MaybeError QueueBase::ValidateWriteBuffer(const BufferBase* buffer,
                                              uint64_t bufferOffset,
                                              size_t size) const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(GetDevice()->ValidateObject(buffer));

        if (bufferOffset % 4 != 0) {
            return DAWN_VALIDATION_ERROR("Queue::WriteBuffer bufferOffset must be a multiple of 4");
        }
        if (size % 4 != 0) {
            return DAWN_VALIDATION_ERROR("Queue::WriteBuffer size must be a multiple of 4");
        }

        uint64_t bufferSize = buffer->GetSize();
        if (bufferOffset > bufferSize || size > (bufferSize - bufferOffset)) {
            return DAWN_VALIDATION_ERROR("Queue::WriteBuffer out of range");
        }

        if (!(buffer->GetUsage() & wgpu::BufferUsage::CopyDst)) {
            return DAWN_VALIDATION_ERROR("Buffer needs the CopyDst usage bit");
        }

        DAWN_TRY(buffer->ValidateCanUseOnQueueNow());

        return {};
    }

    MaybeError QueueBase::ValidateWriteTexture(const TextureCopyView* destination,
                                               size_t dataSize,
                                               const TextureDataLayout* dataLayout,
                                               const Extent3D* writeSize) const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

        DAWN_TRY(ValidateTextureCopyView(GetDevice(), *destination, *writeSize));

        if (dataLayout->offset > dataSize) {
            return DAWN_VALIDATION_ERROR("Queue::WriteTexture out of range");
        }

        if (!(destination->texture->GetUsage() & wgpu::TextureUsage::CopyDst)) {
            return DAWN_VALIDATION_ERROR("Texture needs the CopyDst usage bit");
        }

        if (destination->texture->GetSampleCount() > 1) {
            return DAWN_VALIDATION_ERROR("The sample count of textures must be 1");
        }

        DAWN_TRY(ValidateBufferToTextureCopyRestrictions(*destination));
        // We validate texture copy range before validating linear texture data,
        // because in the latter we divide copyExtent.width by blockWidth and
        // copyExtent.height by blockHeight while the divisibility conditions are
        // checked in validating texture copy range.
        DAWN_TRY(ValidateTextureCopyRange(*destination, *writeSize));
        DAWN_TRY(ValidateLinearTextureData(
            *dataLayout, dataSize,
            destination->texture->GetFormat().GetTexelBlockInfo(destination->aspect), *writeSize));

        DAWN_TRY(destination->texture->ValidateCanUseInSubmitNow());

        return {};
    }

    void QueueBase::SubmitInternal(uint32_t commandCount, CommandBufferBase* const* commands) {
        DeviceBase* device = GetDevice();
        if (device->ConsumedError(device->ValidateIsAlive())) {
            // If device is lost, don't let any commands be submitted
            return;
        }

        TRACE_EVENT0(device->GetPlatform(), General, "Queue::Submit");
        if (device->IsValidationEnabled() &&
            device->ConsumedError(ValidateSubmit(commandCount, commands))) {
            return;
        }
        ASSERT(!IsError());

        if (device->ConsumedError(SubmitImpl(commandCount, commands))) {
            return;
        }

        device->GetErrorScopeTracker()->TrackUntilLastSubmitComplete(
            device->GetCurrentErrorScope());
    }

    void CopyTextureData(uint8_t* dstPointer,
                         const uint8_t* srcPointer,
                         uint32_t depth,
                         uint32_t rowsPerImageInBlock,
                         uint64_t imageAdditionalStride,
                         uint32_t actualBytesPerRow,
                         uint32_t dstBytesPerRow,
                         uint32_t srcBytesPerRow) {
        bool copyWholeLayer =
            actualBytesPerRow == dstBytesPerRow && dstBytesPerRow == srcBytesPerRow;
        bool copyWholeData = copyWholeLayer && imageAdditionalStride == 0;

        if (!copyWholeLayer) {  // copy row by row
            for (uint32_t d = 0; d < depth; ++d) {
                for (uint32_t h = 0; h < rowsPerImageInBlock; ++h) {
                    memcpy(dstPointer, srcPointer, actualBytesPerRow);
                    dstPointer += dstBytesPerRow;
                    srcPointer += srcBytesPerRow;
                }
                srcPointer += imageAdditionalStride;
            }
        } else {
            uint64_t layerSize = uint64_t(rowsPerImageInBlock) * actualBytesPerRow;
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
}  // namespace dawn_native
