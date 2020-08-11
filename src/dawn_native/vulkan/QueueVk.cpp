// Copyright 2018 The Dawn Authors
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

#include "dawn_native/vulkan/QueueVk.h"

#include "common/Math.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/vulkan/CommandBufferVk.h"
#include "dawn_native/vulkan/CommandRecordingContext.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

namespace dawn_native { namespace vulkan {

    namespace {
        ResultOrError<UploadHandle> UploadTextureDataAligningBytesPerRow(
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
                ToBackend(device)
                    ->GetDeviceInfo()
                    .properties.limits.optimalBufferCopyOffsetAlignment;

            UploadHandle uploadHandle;
            DAWN_TRY_ASSIGN(uploadHandle, device->GetDynamicUploader()->Allocate(
                                              newDataSizeBytes + optimalOffsetAlignment - 1,
                                              device->GetPendingCommandSerial()));
            ASSERT(uploadHandle.mappedBuffer != nullptr);

            uint8_t* dstPointer = static_cast<uint8_t*>(uploadHandle.mappedBuffer);
            const uint8_t* srcPointer = static_cast<const uint8_t*>(data);
            srcPointer += dataLayout.offset;

            uint32_t alignedRowsPerImageInBlock = alignedRowsPerImage / blockInfo.blockHeight;
            uint32_t dataRowsPerImageInBlock = dataLayout.rowsPerImage / blockInfo.blockHeight;
            if (dataRowsPerImageInBlock == 0) {
                dataRowsPerImageInBlock = writeSizePixel.height / blockInfo.blockHeight;
            }

            uint64_t additionalOffset =
                Align(uploadHandle.startOffset, optimalOffsetAlignment) - uploadHandle.startOffset;
            uploadHandle.startOffset += additionalOffset;
            dstPointer += additionalOffset;

            ASSERT(dataRowsPerImageInBlock >= alignedRowsPerImageInBlock);
            uint64_t imageAdditionalStride =
                dataLayout.bytesPerRow * (dataRowsPerImageInBlock - alignedRowsPerImageInBlock);

            CopyTextureData(dstPointer, srcPointer, writeSizePixel.depth,
                            alignedRowsPerImageInBlock, imageAdditionalStride, alignedBytesPerRow,
                            optimallyAlignedBytesPerRow, dataLayout.bytesPerRow);

            return uploadHandle;
        }
    }  // namespace

    // static
    Queue* Queue::Create(Device* device) {
        return new Queue(device);
    }

    Queue::~Queue() {
    }

    MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
        Device* device = ToBackend(GetDevice());

        device->Tick();

        TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording,
                           "CommandBufferVk::RecordCommands");
        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(ToBackend(commands[i])->RecordCommands(recordingContext));
        }
        TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferVk::RecordCommands");

        DAWN_TRY(device->SubmitPendingCommands());

        return {};
    }

    MaybeError Queue::WriteTextureImpl(const TextureCopyView& destination,
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

        uint32_t optimalBytesPerRowAlignment =
            ToBackend(GetDevice())
                ->GetDeviceInfo()
                .properties.limits.optimalBufferCopyRowPitchAlignment;
        uint32_t optimallyAlignedBytesPerRow =
            Align(alignedBytesPerRow, optimalBytesPerRowAlignment);

        UploadHandle uploadHandle;
        DAWN_TRY_ASSIGN(uploadHandle,
                        UploadTextureDataAligningBytesPerRow(
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

        return ToBackend(GetDevice())
            ->CopyFromStagingToTexture(uploadHandle.stagingBuffer, passDataLayout, &textureCopy,
                                       writeSizePixel);
    }
}}  // namespace dawn_native::vulkan
