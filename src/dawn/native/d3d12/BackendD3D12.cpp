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

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/PlatformFunctions.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

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

    // Check if DXC is available and cache DXC version information
    if (!mFunctions->IsDXCBinaryAvailable()) {
        // DXC version information is not available if DXC binaries are not available.
        mDxcVersionInfo = DxcUnavailable{"DXC binary is not available"};
    } else {
        // Check the DXC version information and validate them being not lower than pre-defined
        // minimum version.
        AcquireDxcVersionInformation();

        // Check that DXC version information is acquired successfully.
        if (std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo)) {
            const DxcVersionInfo& dxcVersionInfo = std::get<DxcVersionInfo>(mDxcVersionInfo);

            // The required minimum version for DXC compiler and validator.
            // Notes about requirement consideration:
            //   * DXC version 1.4 has some known issues when compiling Tint generated HLSL program,
            //   please
            //     refer to crbug.com/tint/1719
            //   * Windows SDK 20348 provides DXC compiler and validator version 1.6
            // Here the minimum version requirement for DXC compiler and validator are both set
            // to 1.6.
            constexpr uint64_t minimumCompilerMajorVersion = 1;
            constexpr uint64_t minimumCompilerMinorVersion = 6;
            constexpr uint64_t minimumValidatorMajorVersion = 1;
            constexpr uint64_t minimumValidatorMinorVersion = 6;

            // Check that DXC compiler and validator version are not lower than minimum.
            if (dxcVersionInfo.DxcCompilerVersion <
                    MakeDXCVersion(minimumCompilerMajorVersion, minimumCompilerMinorVersion) ||
                dxcVersionInfo.DxcValidatorVersion <
                    MakeDXCVersion(minimumValidatorMajorVersion, minimumValidatorMinorVersion)) {
                // If DXC version is lower than required minimum, set mDxcVersionInfo to
                // DxcUnavailable to indicate that DXC is not available.
                std::ostringstream ss;
                ss << "DXC version too low: dxil.dll required version 1.6, actual version "
                   << (dxcVersionInfo.DxcValidatorVersion >> 32) << "."
                   << (dxcVersionInfo.DxcValidatorVersion & ((uint64_t(1) << 32) - 1))
                   << ", dxcompiler.dll required version 1.6, actual version "
                   << (dxcVersionInfo.DxcCompilerVersion >> 32) << "."
                   << (dxcVersionInfo.DxcCompilerVersion & ((uint64_t(1) << 32) - 1));
                mDxcVersionInfo = DxcUnavailable{ss.str()};
            }
        }
    }

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

void Backend::AcquireDxcVersionInformation() {
    ASSERT(std::holds_alternative<DxcUnavailable>(mDxcVersionInfo));

    auto tryAcquireDxcVersionInfo = [this]() -> ResultOrError<DxcVersionInfo> {
        DAWN_TRY(EnsureDxcValidator());
        DAWN_TRY(EnsureDxcCompiler());

        ComPtr<IDxcVersionInfo> compilerVersionInfo;

        DAWN_TRY(CheckHRESULT(mDxcCompiler.As(&compilerVersionInfo),
                              "D3D12 QueryInterface IDxcCompiler to IDxcVersionInfo"));
        uint32_t compilerMajor, compilerMinor;
        DAWN_TRY(CheckHRESULT(compilerVersionInfo->GetVersion(&compilerMajor, &compilerMinor),
                              "IDxcVersionInfo::GetVersion"));

        ComPtr<IDxcVersionInfo> validatorVersionInfo;

        DAWN_TRY(CheckHRESULT(mDxcValidator.As(&validatorVersionInfo),
                              "D3D12 QueryInterface IDxcValidator to IDxcVersionInfo"));
        uint32_t validatorMajor, validatorMinor;
        DAWN_TRY(CheckHRESULT(validatorVersionInfo->GetVersion(&validatorMajor, &validatorMinor),
                              "IDxcVersionInfo::GetVersion"));

        // Pack major and minor version number into a single version number.
        uint64_t compilerVersion = MakeDXCVersion(compilerMajor, compilerMinor);
        uint64_t validatorVersion = MakeDXCVersion(validatorMajor, validatorMinor);
        return DxcVersionInfo{compilerVersion, validatorVersion};
    };

    auto dxcVersionInfoOrError = tryAcquireDxcVersionInfo();

    if (dxcVersionInfoOrError.IsSuccess()) {
        // Cache the DXC version information.
        mDxcVersionInfo = dxcVersionInfoOrError.AcquireSuccess();
    } else {
        // Error occurs when acquiring DXC version information, set the cache to unavailable and
        // record the error message.
        std::string errorMessage = dxcVersionInfoOrError.AcquireError()->GetFormattedMessage();
        dawn::ErrorLog() << errorMessage;
        mDxcVersionInfo = DxcUnavailable{errorMessage};
    }
}

// Return both DXC compiler and DXC validator version, assert that DXC version information is
// acquired succesfully.
DxcVersionInfo Backend::GetDxcVersion() const {
    ASSERT(std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo));
    return DxcVersionInfo(std::get<DxcVersionInfo>(mDxcVersionInfo));
}

// Return true if and only if DXC binary is avaliable, and the DXC version is validated to
// be no older than a pre-defined minimum version.
bool Backend::IsDXCAvailable() const {
    // mDxcVersionInfo hold DxcVersionInfo instead of DxcUnavailable if and only if DXC binaries and
    // version are validated in `Initialize`.
    return std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo);
}

// Return true if and only if IsDXCAvailable() return true, and the DXC compiler and validator
// version are validated to be no older than the minimium version given in parameter.
bool Backend::IsDXCAvailableAndVersionAtLeast(uint64_t minimumCompilerMajorVersion,
                                              uint64_t minimumCompilerMinorVersion,
                                              uint64_t minimumValidatorMajorVersion,
                                              uint64_t minimumValidatorMinorVersion) const {
    // mDxcVersionInfo hold DxcVersionInfo instead of DxcUnavailable if and only if DXC binaries and
    // version are validated in `Initialize`.
    if (std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo)) {
        const DxcVersionInfo& dxcVersionInfo = std::get<DxcVersionInfo>(mDxcVersionInfo);
        // Check that DXC compiler and validator version are not lower than given requirements.
        if (dxcVersionInfo.DxcCompilerVersion >=
                MakeDXCVersion(minimumCompilerMajorVersion, minimumCompilerMinorVersion) &&
            dxcVersionInfo.DxcValidatorVersion >=
                MakeDXCVersion(minimumValidatorMajorVersion, minimumValidatorMinorVersion)) {
            return true;
        }
    }
    return false;
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
