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

#ifndef SRC_DAWN_NATIVE_D3D12_D3D12_PLATFORM_H_
#define SRC_DAWN_NATIVE_D3D12_D3D12_PLATFORM_H_

// Pre-emptively include windows.h but remove its macros so that they aren't set when declaring the
// COM interfaces. Otherwise ID3D12InfoQueue::GetMessage would be either GetMessageA or GetMessageW
// which causes compilation errors.
// NOLINTNEXTLINE(build/include_order)
#include "dawn/common/windows_with_undefs.h"

#include <d3d11_2.h>    // NOLINT(build/include_order)
#include <d3d11on12.h>  // NOLINT(build/include_order)
#include <d3d12.h>      // NOLINT(build/include_order)
#include <dxcapi.h>     // NOLINT(build/include_order)
#include <dxgi1_4.h>    // NOLINT(build/include_order)
#include <wrl.h>        // NOLINT(build/include_order)

// DXProgrammableCapture.h takes a dependency on other platform header
// files, so it must be defined after them.
#include <DXProgrammableCapture.h>  // NOLINT(build/include_order)
#include <dxgidebug.h>              // NOLINT(build/include_order)

using Microsoft::WRL::ComPtr;

#endif  // SRC_DAWN_NATIVE_D3D12_D3D12_PLATFORM_H_
