// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D_BACKENDD3D_H_
#define SRC_DAWN_NATIVE_D3D_BACKENDD3D_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "dawn/common/TypedInteger.h"
#include "dawn/native/BackendConnection.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d {

class PlatformFunctions;

// DxcVersionInfo holds both DXC compiler (dxcompiler.dll) version and DXC validator (dxil.dll)
// version, which are not necessarily identical. Both are in uint64_t type, as the result of
// MakeDXCVersion.
struct DxcVersionInfo {
    uint64_t DxcCompilerVersion;
    uint64_t DxcValidatorVersion;
};

// If DXC version information is not avaliable due to no DXC binary or error occurs when acquiring
// version, DxcUnavailable indicates the version information being unavailable and holds the
// detailed error information.
struct DxcUnavailable {
    std::string ErrorMessage;
};

class Backend : public BackendConnection {
  public:
    Backend(InstanceBase* instance, wgpu::BackendType type);

    MaybeError Initialize(std::unique_ptr<PlatformFunctions> functions);

    IDXGIFactory4* GetFactory() const;

    MaybeError EnsureDxcLibrary();
    MaybeError EnsureDxcCompiler();
    MaybeError EnsureDxcValidator();
    ComPtr<IDxcLibrary> GetDxcLibrary() const;
    ComPtr<IDxcCompiler3> GetDxcCompiler() const;
    ComPtr<IDxcValidator> GetDxcValidator() const;

    // Return true if and only if DXC binary is avaliable, and the DXC compiler and validator
    // version are validated to be no older than a specific minimium version, currently 1.6.
    bool IsDXCAvailable() const;

    // Return true if and only if mIsDXCAvailable is true, and the DXC compiler and validator
    // version are validated to be no older than the minimium version given in parameter.
    bool IsDXCAvailableAndVersionAtLeast(uint64_t minimumCompilerMajorVersion,
                                         uint64_t minimumCompilerMinorVersion,
                                         uint64_t minimumValidatorMajorVersion,
                                         uint64_t minimumValidatorMinorVersion) const;

    // Return the DXC version information cached in mDxcVersionInformation, assert that the version
    // information is valid. Must be called after ensuring `IsDXCAvailable()` return true.
    DxcVersionInfo GetDxcVersion() const;

    const PlatformFunctions* GetFunctions() const;

    std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevices(
        const RequestAdapterOptions* options) override;
    void ClearPhysicalDevices() override;
    size_t GetPhysicalDeviceCountForTesting() const override;

  protected:
    virtual ResultOrError<Ref<PhysicalDeviceBase>> CreatePhysicalDeviceFromIDXGIAdapter(
        ComPtr<IDXGIAdapter> dxgiAdapter) = 0;

  private:
    ResultOrError<Ref<PhysicalDeviceBase>> GetOrCreatePhysicalDeviceFromLUID(LUID luid);
    ResultOrError<Ref<PhysicalDeviceBase>> GetOrCreatePhysicalDeviceFromIDXGIAdapter(
        ComPtr<IDXGIAdapter> dxgiAdapter);

    // Acquiring DXC version information and store the result in mDxcVersionInfo. This function
    // should be called only once, during startup in `Initialize`.
    void AcquireDxcVersionInformation();

    // Keep mFunctions as the first member so that in the destructor it is freed last. Otherwise
    // the D3D12 DLLs are unloaded before we are done using them.
    std::unique_ptr<PlatformFunctions> mFunctions;
    ComPtr<IDXGIFactory4> mFactory;
    ComPtr<IDxcLibrary> mDxcLibrary;
    ComPtr<IDxcCompiler3> mDxcCompiler;
    ComPtr<IDxcValidator> mDxcValidator;

    // DXC binaries and DXC version information are checked when start up in `Initialize`. There are
    // two possible states:
    //   1. The DXC binary is not available, or error occurs when checking the version information
    //      and therefore no DXC version information available, or the DXC version is lower than
    //      requested minumum and therefore DXC is not available, represented by DxcUnavailable
    //      in which a error message is held;
    //   3. The DXC version information is acquired successfully and validated not lower than
    //      requested minimum, stored in DxcVersionInfo.
    std::variant<DxcUnavailable, DxcVersionInfo> mDxcVersionInfo;

    struct LUIDHashFunc {
        size_t operator()(const LUID& luid) const;
    };
    struct LUIDEqualFunc {
        bool operator()(const LUID& a, const LUID& b) const;
    };

    // Map of LUID to physical device.
    // The LUID is guaranteed to be uniquely identify an adapter on the local
    // machine until restart.
    std::unordered_map<LUID, Ref<PhysicalDeviceBase>, LUIDHashFunc, LUIDEqualFunc> mPhysicalDevices;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_BACKENDD3D_H_
