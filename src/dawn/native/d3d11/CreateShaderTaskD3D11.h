// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_CREATESHADERTASKD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_CREATESHADERTASKD3D11_H_

#include <functional>
#include <string>
#include <utility>

#include "dawn/common/Ref.h"
#include "dawn/common/Sha3.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/native/FutureResult.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/ShaderUtils.h"
#include "dawn/native/d3d/d3d_platform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::platform {
class Platform;
}

namespace dawn::native::d3d11 {

template <typename T>
using FutureComPtr = FutureResult<ComPtr<T>>;

namespace detail {

template <SingleShaderStage kShaderStage>
struct CreateShaderTaskTraits {
    using ComType = void;
};

template <>
struct CreateShaderTaskTraits<SingleShaderStage::Vertex> {
    using ComType = ID3D11VertexShader;
    static constexpr char kTraceLabel[] = "DeviceD3D11::CreateVertexShader";
    static constexpr char kHistogramLabel[] = "D3D11.CreateVertexShaderUs";

    template <typename... Args>
    static MaybeError CreateShader(const ComPtr<ID3D11Device>& d3d11Device, Args&&... args) {
        return CheckHRESULT(d3d11Device->CreateVertexShader(std::forward<Args>(args)...),
                            "D3D11 create vertex shader");
    }
};

template <>
struct CreateShaderTaskTraits<SingleShaderStage::Fragment> {
    using ComType = ID3D11PixelShader;
    static constexpr char kTraceLabel[] = "DeviceD3D11::CreatePixelShader";
    static constexpr char kHistogramLabel[] = "D3D11.CreatePixelShaderUs";

    template <typename... Args>
    static MaybeError CreateShader(const ComPtr<ID3D11Device>& d3d11Device, Args&&... args) {
        return CheckHRESULT(d3d11Device->CreatePixelShader(std::forward<Args>(args)...),
                            "D3D11 create pixel shader");
    }
};

template <>
struct CreateShaderTaskTraits<SingleShaderStage::Compute> {
    using ComType = ID3D11ComputeShader;
    static constexpr char kTraceLabel[] = "DeviceD3D11::CreateComputeShader";
    static constexpr char kHistogramLabel[] = "D3D11.CreateComputeShaderUs";

    template <typename... Args>
    static MaybeError CreateShader(const ComPtr<ID3D11Device>& d3d11Device, Args&&... args) {
        return CheckHRESULT(d3d11Device->CreateComputeShader(std::forward<Args>(args)...),
                            "D3D11 create compute shader");
    }
};

}  // namespace detail

template <SingleShaderStage kShaderStage>
std::function<void()> CreateShaderTask(
    ComPtr<ID3D11Device> d3d11Device,
    dawn::platform::Platform* platform,
    d3d::CompiledShader&& compiledShader,
    Ref<FutureComPtr<typename detail::CreateShaderTaskTraits<kShaderStage>::ComType>> future,
    std::string label) {
    using Traits = detail::CreateShaderTaskTraits<kShaderStage>;
    using ShaderComType = typename Traits::ComType;

    // RefCounted Wrapper for d3d::CompiledShader since the latter is not copyable. Thus cannot be
    // bound to a std::function.
    class ShaderBlobWrapper : public RefCounted {
      public:
        explicit ShaderBlobWrapper(d3d::CompiledShader&& compiledShader)
            : compiledShader(std::move(compiledShader)) {}

        const void* Data() const { return compiledShader.shaderBlob.Data(); }
        size_t Size() const { return compiledShader.shaderBlob.Size(); }

      private:
        d3d::CompiledShader compiledShader;
    };

    auto shaderBlob = AcquireRef(new ShaderBlobWrapper(std::move(compiledShader)));

    auto CreateShaderOrError = [=]() -> ResultOrError<ComPtr<ShaderComType>> {
        ComPtr<ShaderComType> shader;
        DAWN_TRY(Traits::CreateShader(d3d11Device, shaderBlob->Data(), shaderBlob->Size(), nullptr,
                                      &shader));
        return shader;
    };

    return [=, label = std::move(label)]() {
        TRACE_EVENT1(platform, General, Traits::kTraceLabel, "label", label.c_str());
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(platform, Traits::kHistogramLabel);

        future->Set(CreateShaderOrError());
    };
}

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_CREATESHADERTASKD3D11_H_
