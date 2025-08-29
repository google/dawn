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

#ifndef SRC_DAWN_NATIVE_DYNAMICARRAYSTATE_H_
#define SRC_DAWN_NATIVE_DYNAMICARRAYSTATE_H_

#include <vector>

#include "dawn/common/Ref.h"
#include "dawn/common/ityp_span.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// An optional component of a BindGroup that's used to track the resources that are in the dynamic
// binding array part. It helps maintain the metadata buffer that's used in shaders to know if it is
// valid to access an entry of the dynamic binding array with a given type (note that the writing of
// the updates to the buffer are done by the backends).
class DynamicArrayState {
  public:
    explicit DynamicArrayState(BindingIndex size);

    MaybeError Initialize(DeviceBase* device);

    BindingIndex GetSize() const;
    ityp::span<BindingIndex, const Ref<TextureViewBase>> GetBindings() const;
    BufferBase* GetMetadataBuffer() const;
    bool IsDestroyed() const;

    void Update(BindingIndex i, TextureViewBase* view);
    void Destroy();

  private:
    bool mDestroyed = false;
    ityp::vector<BindingIndex, Ref<TextureViewBase>> mBindings;
    Ref<BufferBase> mMetadataBuffer;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DYNAMICARRAYSTATE_H_
