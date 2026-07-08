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

#ifndef SRC_DAWN_NATIVE_IMMEDIATESTRACKER_H_
#define SRC_DAWN_NATIVE_IMMEDIATESTRACKER_H_

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>

#include "ImmediatesLayout.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/ityp_bitset.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/ImmediatesLayout.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/Pipeline.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn::native {

template <typename T>
struct ImmediateDataContent {
  public:
    const T* operator->() const { return reinterpret_cast<const T*>(&mData); }
    T* operator->() { return reinterpret_cast<T*>(&mData); }

    const unsigned char* data() const { return mData.data(); }

    template <typename Out>
    const Out* Get(uint32_t offset) const {
        DAWN_ASSERT(sizeof(Out) + offset <= sizeof(T));
        return reinterpret_cast<const Out*>(&DAWN_UNSAFE_TODO(mData[offset]));
    }

    template <typename Out>
    Out* Get(uint32_t offset) {
        DAWN_ASSERT(sizeof(Out) + offset <= sizeof(T));
        return reinterpret_cast<Out*>(&DAWN_UNSAFE_TODO(mData[offset]));
    }

  private:
    std::array<unsigned char, sizeof(T)> mData = {0};
};

// TODO(crbug.com/366291600): Add inheritance ability(like BindGroupTracker) so that it can inherit
// immediates in native backend if supported.
template <typename T, typename PipelineType>
class UserImmediatesTrackerBase {
  public:
    UserImmediatesTrackerBase() {}

    // Setters
    void SetImmediates(uint32_t offset, uint8_t* values, uint32_t size) {
        uint8_t* destData = mContent.template Get<uint8_t>(offsetof(T, userImmediates) + offset);
        if (DAWN_UNSAFE_TODO(memcmp(destData, values, size)) != 0) {
            DAWN_UNSAFE_TODO(memcpy(destData, values, size));
            mDirty |= GetImmediateBlockBits(offsetof(T, userImmediates), sizeof(UserImmediates));
        }
    }

    // TODO(crbug.com/366291600): Support immediate data compatible.
    void OnSetPipeline(PipelineType* pipeline) {
        if (mLastPipeline == pipeline) {
            return;
        }

        mDirty = pipeline->GetImmediateMask();
        mLastPipeline = pipeline;
    }

    // Getters
    const ImmediateMask& GetDirtyBits() const { return mDirty; }

    const ImmediateDataContent<T>& GetContent() const { return mContent; }

    void SetDirtyBitsForTesting(ImmediateMask dirtyBits) { mDirty = dirtyBits; }

  protected:
    template <typename U>
    void UpdateImmediates(size_t dataOffset, const U& data) {
        constexpr size_t dataSize = sizeof(U);
        U* destData = mContent.template Get<U>(uint32_t(dataOffset));
        if (DAWN_UNSAFE_TODO(memcmp(destData, &data, dataSize)) != 0) {
            DAWN_UNSAFE_TODO(memcpy(destData, &data, dataSize));
            mDirty |= GetImmediateBlockBits(dataOffset, dataSize);
        }
    }

    ImmediateDataContent<T> mContent;
    ImmediateMask mDirty = ImmediateMask(0);
    RAW_PTR_EXCLUSION PipelineType* mLastPipeline = nullptr;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_IMMEDIATESTRACKER_H_
