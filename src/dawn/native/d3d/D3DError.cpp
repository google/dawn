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

#include "dawn/native/d3d/D3DError.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace dawn::native::d3d {
const char* HRESULTAsString(HRESULT result) {
    // There's a lot of possible HRESULTS, but these ones are the ones specifically listed as
    // being returned from D3D11 and D3D12, in addition to fake codes used internally for testing.
    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/d3d12-graphics-reference-returnvalues
    switch (result) {
        case S_OK:
            return "S_OK";
        case S_FALSE:
            return "S_FALSE";

        // Generic errors:
        case E_FAIL:
            return "E_FAIL";
        case E_INVALIDARG:
            return "E_INVALIDARG";
        case E_OUTOFMEMORY:
            return "E_OUTOFMEMORY";
        case E_NOTIMPL:
            return "E_NOTIMPL";

        // DXGI errors:
        case DXGI_ERROR_INVALID_CALL:
            return "DXGI_ERROR_INVALID_CALL";
        case DXGI_ERROR_UNSUPPORTED:
            return "DXGI_ERROR_UNSUPPORTED";
        case DXGI_ERROR_DEVICE_REMOVED:
            return "DXGI_ERROR_DEVICE_REMOVED";
        case DXGI_ERROR_DEVICE_HUNG:
            return "DXGI_ERROR_DEVICE_HUNG";
        case DXGI_ERROR_DEVICE_RESET:
            return "DXGI_ERROR_DEVICE_RESET";
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
            return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
        case DXGI_ERROR_WAS_STILL_DRAWING:
            return "DXGI_ERROR_WAS_STILL_DRAWING";

        // D3D11 errors:
        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
        case D3D11_ERROR_FILE_NOT_FOUND:
            return "D3D11_ERROR_FILE_NOT_FOUND";
        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
            return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";

        // D3D12 errors:
        case D3D12_ERROR_ADAPTER_NOT_FOUND:
            return "D3D12_ERROR_ADAPTER_NOT_FOUND";
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH:
            return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";

        // Fake errors used for testing:
        case E_FAKE_ERROR_FOR_TESTING:
            return "E_FAKE_ERROR_FOR_TESTING";
        case E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING:
            return "E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING";

        default:
            return "<Unknown HRESULT>";
    }
}

MaybeError CheckHRESULTImpl(HRESULT result, const char* context) {
    if (DAWN_LIKELY(SUCCEEDED(result))) {
        return {};
    }

    std::ostringstream messageStream;
    messageStream << context << " failed with " << HRESULTAsString(result) << " (0x"
                  << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << result
                  << ")";

    if (result == DXGI_ERROR_DEVICE_REMOVED) {
        return DAWN_DEVICE_LOST_ERROR(messageStream.str());
    } else {
        return DAWN_INTERNAL_ERROR(messageStream.str());
    }
}

MaybeError CheckOutOfMemoryHRESULTImpl(HRESULT result, const char* context) {
    if (result == E_OUTOFMEMORY || result == E_FAKE_OUTOFMEMORY_ERROR_FOR_TESTING) {
        return DAWN_OUT_OF_MEMORY_ERROR(context);
    }

    return CheckHRESULTImpl(result, context);
}

}  // namespace dawn::native::d3d
