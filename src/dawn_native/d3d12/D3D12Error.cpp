// Copyright 2019 The Dawn Authors
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

#include "dawn_native/d3d12/D3D12Error.h"

#include <string>

namespace dawn_native { namespace d3d12 {
    MaybeError CheckHRESULTImpl(HRESULT result, const char* context) {
        if (DAWN_LIKELY(SUCCEEDED(result))) {
            return {};
        }

        std::string message = std::string(context) + " failed with " + std::to_string(result);

        if (result == DXGI_ERROR_DEVICE_REMOVED) {
            return DAWN_DEVICE_LOST_ERROR(message);
        } else {
            return DAWN_INTERNAL_ERROR(message);
        }
    }

    MaybeError CheckOutOfMemoryHRESULTImpl(HRESULT result, const char* context) {
        if (result == E_OUTOFMEMORY || result == E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING) {
            return DAWN_OUT_OF_MEMORY_ERROR(context);
        }

        return CheckHRESULTImpl(result, context);
    }

}}  // namespace dawn_native::d3d12
