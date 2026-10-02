// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_BUFFERTEXTURECOPYOVERSIZEDROW_H_
#define SRC_DAWN_NATIVE_BUFFERTEXTURECOPYOVERSIZEDROW_H_

#include "src/dawn/native/BlockInfo.h"
#include "src/dawn/native/Commands.h"

namespace dawn::native {

class CommandAllocator;
class DeviceBase;

// Determines whether a buffer<->texture copy needs to be split for oversized rows. This happens
// if the SplitBufferTextureCopyForOversizedRow toggle is enabled and `bufferCopy`'s bytes per row
// or texels per row overflows the limited-width hardware register field used to store it,
// respectively. See https://crbug.com/481934465.
bool NeedsBufferTextureCopySplitForOversizedRow(const DeviceBase* device,
                                                const BufferCopy& bufferCopy,
                                                const TypedTexelBlockInfo& blockInfo);

// Applies the oversized-row-pitch workaround to a texture-to-buffer copy: splits
// {bufferCopy, textureCopy, copySize} into one sub-copy per unpadded row and allocates one
// CopyTextureToBufferCmd per sub-copy into `allocator`. Callers should only call this when
// NeedsBufferTextureCopySplitForOversizedRow returns true.
void SplitCopyTextureToBufferForOversizedRow(CommandAllocator* allocator,
                                             const BufferCopy& bufferCopy,
                                             const TextureCopy& textureCopy,
                                             const BlockExtent3D& copySize,
                                             const TypedTexelBlockInfo& blockInfo);

// Same as SplitCopyTextureToBufferForOversizedRow but for the buffer-to-texture direction:
// allocates one CopyBufferToTextureCmd per sub-copy into `allocator`.
void SplitCopyBufferToTextureForOversizedRow(CommandAllocator* allocator,
                                             const BufferCopy& bufferCopy,
                                             const TextureCopy& textureCopy,
                                             const BlockExtent3D& copySize,
                                             const TypedTexelBlockInfo& blockInfo);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUFFERTEXTURECOPYOVERSIZEDROW_H_
