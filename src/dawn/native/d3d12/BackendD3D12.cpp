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

#include "dawn/native/d3d12/BackendD3D12.h"

#include <utility>

#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/PlatformFunctions.h"

namespace dawn::native::d3d12 {

namespace {

ResultOrError<ComPtr<IDXGIFactory4>> CreateFactory(const PlatformFunctions* functions,
                                                   BackendValidationLevel validationLevel,
                                                   bool beginCaptureOnStartup) {
    ComPtr<IDXGIFactory4> factory;

    uint32_t dxgiFactoryFlags = 0;

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    {
        if (validationLevel != BackendValidationLevel::Disabled) {
            ComPtr<ID3D12Debug3> debugController;
            if (SUCCEEDED(functions->d3d12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                ASSERT(debugController != nullptr);
                debugController->EnableDebugLayer();
                if (validationLevel == BackendValidationLevel::Full) {
                    debugController->SetEnableGPUBasedValidation(true);
                }

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }

        if (beginCaptureOnStartup) {
            ComPtr<IDXGraphicsAnalysis> graphicsAnalysis;
            if (functions->dxgiGetDebugInterface1 != nullptr &&
                SUCCEEDED(functions->dxgiGetDebugInterface1(0, IID_PPV_ARGS(&graphicsAnalysis)))) {
                graphicsAnalysis->BeginCapture();
            }
        }
    }

    if (FAILED(functions->createDxgiFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))) {
        return DAWN_INTERNAL_ERROR("Failed to create a DXGI factory");
    }

    ASSERT(factory != nullptr);
    return std::move(factory);
}

ResultOrError<Ref<AdapterBase>> CreateAdapterFromIDXGIAdapter(Backend* backend,
                                                              ComPtr<IDXGIAdapter> dxgiAdapter) {
    ComPtr<IDXGIAdapter3> dxgiAdapter3;
    DAWN_TRY(CheckHRESULT(dxgiAdapter.As(&dxgiAdapter3), "DXGIAdapter retrieval"));
    Ref<Adapter> adapter = AcquireRef(new Adapter(backend, std::move(dxgiAdapter3)));
    DAWN_TRY(adapter->Initialize());

    return {std::move(adapter)};
}

}  // anonymous namespace

Backend::Backend(InstanceBase* instance) : BackendConnection(instance, wgpu::BackendType::D3D12) {}

MaybeError Backend::Initialize() {
    mFunctions = std::make_unique<PlatformFunctions>();
    DAWN_TRY(mFunctions->LoadFunctions());

    const auto instance = GetInstance();

    DAWN_TRY_ASSIGN(mFactory, CreateFactory(mFunctions.get(), instance->GetBackendValidationLevel(),
                                            instance->IsBeginCaptureOnStartupEnabled()));

    return {};
}

ComPtr<IDXGIFactory4> Backend::GetFactory() const {
    return mFactory;
}

MaybeError Backend::EnsureDxcLibrary() {
    if (mDxcLibrary == nullptr) {
        DAWN_TRY(CheckHRESULT(
            mFunctions->dxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&mDxcLibrary)),
            "DXC create library"));
        ASSERT(mDxcLibrary != nullptr);
    }
    return {};
}

MaybeError Backend::EnsureDxcCompiler() {
    if (mDxcCompiler == nullptr) {
        DAWN_TRY(CheckHRESULT(
            mFunctions->dxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mDxcCompiler)),
            "DXC create compiler"));
        ASSERT(mDxcCompiler != nullptr);
    }
    return {};
}

MaybeError Backend::EnsureDxcValidator() {
    if (mDxcValidator == nullptr) {
        DAWN_TRY(CheckHRESULT(
            mFunctions->dxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&mDxcValidator)),
            "DXC create validator"));
        ASSERT(mDxcValidator != nullptr);
    }
    return {};
}

ComPtr<IDxcLibrary> Backend::GetDxcLibrary() const {
    ASSERT(mDxcLibrary != nullptr);
    return mDxcLibrary;
}

ComPtr<IDxcCompiler> Backend::GetDxcCompiler() const {
    ASSERT(mDxcCompiler != nullptr);
    return mDxcCompiler;
}

ComPtr<IDxcValidator> Backend::GetDxcValidator() const {
    ASSERT(mDxcValidator != nullptr);
    return mDxcValidator;
}

const PlatformFunctions* Backend::GetFunctions() const {
    return mFunctions.get();
}

std::vector<Ref<AdapterBase>> Backend::DiscoverDefaultAdapters() {
    AdapterDiscoveryOptions options;
    auto result = DiscoverAdapters(&options);
    if (result.IsError()) {
        GetInstance()->ConsumedError(result.AcquireError());
        return {};
    }
    return result.AcquireSuccess();
}

ResultOrError<std::vector<Ref<AdapterBase>>> Backend::DiscoverAdapters(
    const AdapterDiscoveryOptionsBase* optionsBase) {
    ASSERT(optionsBase->backendType == WGPUBackendType_D3D12);
    const AdapterDiscoveryOptions* options =
        static_cast<const AdapterDiscoveryOptions*>(optionsBase);

    std::vector<Ref<AdapterBase>> adapters;
    if (options->dxgiAdapter != nullptr) {
        // |dxgiAdapter| was provided. Discover just that adapter.
        Ref<AdapterBase> adapter;
        DAWN_TRY_ASSIGN(adapter, CreateAdapterFromIDXGIAdapter(this, options->dxgiAdapter));
        adapters.push_back(std::move(adapter));
        return std::move(adapters);
    }

    // Enumerate and discover all available adapters.
    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (mFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        ASSERT(dxgiAdapter != nullptr);
        ResultOrError<Ref<AdapterBase>> adapter = CreateAdapterFromIDXGIAdapter(this, dxgiAdapter);
        if (adapter.IsError()) {
            GetInstance()->ConsumedError(adapter.AcquireError());
            continue;
        }

        adapters.push_back(adapter.AcquireSuccess());
    }

    return adapters;
}

BackendConnection* Connect(InstanceBase* instance) {
    Backend* backend = new Backend(instance);

    if (instance->ConsumedError(backend->Initialize())) {
        delete backend;
        return nullptr;
    }

    return backend;
}

}  // namespace dawn::native::d3d12
