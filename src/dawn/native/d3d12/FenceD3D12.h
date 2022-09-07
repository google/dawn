// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_FENCED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_FENCED3D12_H_

#include "dawn/common/RefCounted.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Fence : public RefCounted {
  public:
    static ResultOrError<Ref<Fence>> CreateFromHandle(ID3D12Device* device,
                                                      HANDLE unownedHandle,
                                                      UINT64 fenceValue);

    ID3D12Fence* GetD3D12Fence() const;
    UINT64 GetFenceValue() const;

  private:
    Fence(ComPtr<ID3D12Fence> d3d12Fence, UINT64 fenceValue, HANDLE sharedHandle);
    ~Fence() override;

    ComPtr<ID3D12Fence> mD3D12Fence;
    const UINT64 mFenceValue;
    const HANDLE mSharedHandle;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_FENCED3D12_H_
