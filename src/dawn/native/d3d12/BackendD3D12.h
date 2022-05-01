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

#ifndef SRC_DAWN_NATIVE_D3D12_BACKENDD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BACKENDD3D12_H_

#include <memory>
#include <vector>

#include "dawn/native/BackendConnection.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class PlatformFunctions;

class Backend : public BackendConnection {
  public:
    explicit Backend(InstanceBase* instance);

    MaybeError Initialize();

    ComPtr<IDXGIFactory4> GetFactory() const;

    MaybeError EnsureDxcLibrary();
    MaybeError EnsureDxcCompiler();
    MaybeError EnsureDxcValidator();
    ComPtr<IDxcLibrary> GetDxcLibrary() const;
    ComPtr<IDxcCompiler> GetDxcCompiler() const;
    ComPtr<IDxcValidator> GetDxcValidator() const;

    const PlatformFunctions* GetFunctions() const;

    std::vector<Ref<AdapterBase>> DiscoverDefaultAdapters() override;
    ResultOrError<std::vector<Ref<AdapterBase>>> DiscoverAdapters(
        const AdapterDiscoveryOptionsBase* optionsBase) override;

  private:
    // Keep mFunctions as the first member so that in the destructor it is freed last. Otherwise
    // the D3D12 DLLs are unloaded before we are done using them.
    std::unique_ptr<PlatformFunctions> mFunctions;
    ComPtr<IDXGIFactory4> mFactory;
    ComPtr<IDxcLibrary> mDxcLibrary;
    ComPtr<IDxcCompiler> mDxcCompiler;
    ComPtr<IDxcValidator> mDxcValidator;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BACKENDD3D12_H_
