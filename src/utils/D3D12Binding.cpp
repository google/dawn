// Copyright 2017 The NXT Authors
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

#include "utils/BackendBinding.h"

#include "common/Assert.h"
#include "nxt/nxt_wsi.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <initializer_list>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

using Microsoft::WRL::ComPtr;

namespace backend { namespace d3d12 {
    void Init(ComPtr<IDXGIFactory4> factory,
              ComPtr<ID3D12Device> d3d12Device,
              nxtProcTable* procs,
              nxtDevice* device);

    nxtSwapChainImplementation CreateNativeSwapChainImpl(nxtDevice device, HWND window);
    nxtTextureFormat GetNativeSwapChainPreferredFormat(const nxtSwapChainImplementation* swapChain);
}}  // namespace backend::d3d12

namespace utils {
    namespace {
        void ASSERT_SUCCESS(HRESULT hr) {
            ASSERT(SUCCEEDED(hr));
        }

        ComPtr<IDXGIFactory4> CreateFactory() {
            ComPtr<IDXGIFactory4> factory;

            uint32_t dxgiFactoryFlags = 0;
#ifdef _DEBUG
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            // NOTE: Enabling the debug layer after device creation will invalidate the active
            // device.
            {
                ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                    debugController->EnableDebugLayer();

                    // Enable additional debug layers.
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }

                ComPtr<IDXGIDebug1> dxgiDebug;
                if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
                    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                                 DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL));
                }
            }
#endif

            ASSERT_SUCCESS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

            return factory;
        }

    }  // namespace

    class D3D12Binding : public BackendBinding {
      public:
        void SetupGLFWWindowHints() override {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            mFactory = CreateFactory();
            ASSERT(GetHardwareAdapter(mFactory.Get(), &mHardwareAdapter));
            ASSERT_SUCCESS(D3D12CreateDevice(mHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                             IID_PPV_ARGS(&mD3d12Device)));

            backend::d3d12::Init(mFactory, mD3d12Device, procs, device);
            mBackendDevice = *device;
            mProcTable = *procs;
        }

        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                HWND win32Window = glfwGetWin32Window(mWindow);
                mSwapchainImpl =
                    backend::d3d12::CreateNativeSwapChainImpl(mBackendDevice, win32Window);
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }

        nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
            ASSERT(mSwapchainImpl.userData != nullptr);
            return backend::d3d12::GetNativeSwapChainPreferredFormat(&mSwapchainImpl);
        }

      private:
        nxtDevice mBackendDevice = nullptr;
        nxtSwapChainImplementation mSwapchainImpl = {};
        nxtProcTable mProcTable = {};

        // Initialization
        ComPtr<IDXGIFactory4> mFactory;
        ComPtr<IDXGIAdapter1> mHardwareAdapter;
        ComPtr<ID3D12Device> mD3d12Device;

        static bool GetHardwareAdapter(IDXGIFactory4* factory, IDXGIAdapter1** hardwareAdapter) {
            *hardwareAdapter = nullptr;
            for (uint32_t adapterIndex = 0;; ++adapterIndex) {
                IDXGIAdapter1* adapter = nullptr;
                if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND) {
                    break;  // No more adapters to enumerate.
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual
                // device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                                                _uuidof(ID3D12Device), nullptr))) {
                    *hardwareAdapter = adapter;
                    return true;
                }
                adapter->Release();
            }
            return false;
        }
    };

    BackendBinding* CreateD3D12Binding() {
        return new D3D12Binding;
    }

}  // namespace utils
