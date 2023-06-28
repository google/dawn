// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_FENCED3D11_H_
#define SRC_DAWN_NATIVE_D3D11_FENCED3D11_H_

#include "dawn/common/Ref.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/Fence.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Fence : public d3d::Fence {
  public:
    static ResultOrError<Ref<Fence>> CreateFromHandle(ID3D11Device5* device,
                                                      HANDLE unownedHandle,
                                                      UINT64 fenceValue);

    ID3D11Fence* GetD3D11Fence() const;

  private:
    using Base = d3d::Fence;

    Fence(ComPtr<ID3D11Fence> d3d11Fence, UINT64 fenceValue, HANDLE sharedHandle);
    ~Fence() override;

    ComPtr<ID3D11Fence> mD3D11Fence;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_FENCED3D11_H_
