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

#include "dawn/native/d3d12/BlobD3D12.h"

namespace dawn::native {

Blob CreateBlob(ComPtr<ID3DBlob> blob) {
    // Detach so the deleter callback can "own" the reference
    ID3DBlob* ptr = blob.Detach();
    return Blob::UnsafeCreateWithDeleter(reinterpret_cast<uint8_t*>(ptr->GetBufferPointer()),
                                         ptr->GetBufferSize(), [=]() {
                                             // Reattach and drop to delete it.
                                             ComPtr<ID3DBlob> b;
                                             b.Attach(ptr);
                                             b = nullptr;
                                         });
}

Blob CreateBlob(ComPtr<IDxcBlob> blob) {
    // Detach so the deleter callback can "own" the reference
    IDxcBlob* ptr = blob.Detach();
    return Blob::UnsafeCreateWithDeleter(reinterpret_cast<uint8_t*>(ptr->GetBufferPointer()),
                                         ptr->GetBufferSize(), [=]() {
                                             // Reattach and drop to delete it.
                                             ComPtr<IDxcBlob> b;
                                             b.Attach(ptr);
                                             b = nullptr;
                                         });
}

}  // namespace dawn::native
