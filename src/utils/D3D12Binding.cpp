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
#include "utils/SwapChainImpl.h"

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
    void Init(ComPtr<ID3D12Device> d3d12Device, nxtProcTable* procs, nxtDevice* device);
    ComPtr<ID3D12CommandQueue> GetCommandQueue(nxtDevice device);
    uint64_t GetSerial(const nxtDevice device);
    void NextSerial(nxtDevice device);
    void ExecuteCommandLists(nxtDevice device,
                             std::initializer_list<ID3D12CommandList*> commandLists);
    void WaitForSerial(nxtDevice device, uint64_t serial);
    void OpenCommandList(nxtDevice device, ComPtr<ID3D12GraphicsCommandList>* commandList);
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

        DXGI_USAGE D3D12SwapChainBufferUsage(nxtTextureUsageBit allowedUsages) {
            DXGI_USAGE usage = DXGI_CPU_ACCESS_NONE;
            if (allowedUsages & NXT_TEXTURE_USAGE_BIT_SAMPLED) {
                usage |= DXGI_USAGE_SHADER_INPUT;
            }
            if (allowedUsages & NXT_TEXTURE_USAGE_BIT_STORAGE) {
                usage |= DXGI_USAGE_UNORDERED_ACCESS;
            }
            if (allowedUsages & NXT_TEXTURE_USAGE_BIT_OUTPUT_ATTACHMENT) {
                usage |= DXGI_USAGE_RENDER_TARGET_OUTPUT;
            }
            return usage;
        }

        D3D12_RESOURCE_STATES D3D12ResourceState(nxtTextureUsageBit usage) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;
            if (usage & NXT_TEXTURE_USAGE_BIT_TRANSFER_SRC) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & NXT_TEXTURE_USAGE_BIT_TRANSFER_DST) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & NXT_TEXTURE_USAGE_BIT_SAMPLED) {
                resourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
            if (usage & NXT_TEXTURE_USAGE_BIT_STORAGE) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (usage & NXT_TEXTURE_USAGE_BIT_OUTPUT_ATTACHMENT) {
                resourceState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            }

            return resourceState;
        }
    }  // namespace

    class SwapChainImplD3D12 : SwapChainImpl {
      public:
        static nxtSwapChainImplementation Create(HWND window, const nxtProcTable& procs) {
            auto impl = GenerateSwapChainImplementation<SwapChainImplD3D12, nxtWSIContextD3D12>();
            impl.userData = new SwapChainImplD3D12(window, procs);
            return impl;
        }

      private:
        nxtDevice mBackendDevice = nullptr;
        nxtProcTable mProcs = {};

        static constexpr unsigned int kFrameCount = 2;

        HWND mWindow = 0;
        ComPtr<IDXGIFactory4> mFactory = {};
        ComPtr<ID3D12CommandQueue> mCommandQueue = {};
        ComPtr<IDXGISwapChain3> mSwapChain = {};
        ComPtr<ID3D12Resource> mRenderTargetResources[kFrameCount] = {};

        // Frame synchronization. Updated every frame
        uint32_t mRenderTargetIndex = 0;
        uint32_t mPreviousRenderTargetIndex = 0;
        uint64_t mLastSerialRenderTargetWasUsed[kFrameCount] = {};

        D3D12_RESOURCE_STATES mRenderTargetResourceState;

        SwapChainImplD3D12(HWND window, nxtProcTable procs)
            : mWindow(window), mProcs(procs), mFactory(CreateFactory()) {
        }

        ~SwapChainImplD3D12() {
        }

        // For GenerateSwapChainImplementation
        friend class SwapChainImpl;

        void Init(nxtWSIContextD3D12* ctx) {
            mBackendDevice = ctx->device;
            mCommandQueue = backend::d3d12::GetCommandQueue(mBackendDevice);
        }

        nxtSwapChainError Configure(nxtTextureFormat format,
                                    nxtTextureUsageBit allowedUsage,
                                    uint32_t width,
                                    uint32_t height) {
            if (format != NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
                return "unsupported format";
            }
            ASSERT(width > 0);
            ASSERT(height > 0);

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = D3D12SwapChainBufferUsage(allowedUsage);
            swapChainDesc.BufferCount = kFrameCount;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;

            ComPtr<IDXGISwapChain1> swapChain1;
            ASSERT_SUCCESS(mFactory->CreateSwapChainForHwnd(
                mCommandQueue.Get(), mWindow, &swapChainDesc, nullptr, nullptr, &swapChain1));
            ASSERT_SUCCESS(swapChain1.As(&mSwapChain));

            for (uint32_t n = 0; n < kFrameCount; ++n) {
                ASSERT_SUCCESS(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargetResources[n])));
            }

            // Get the initial render target and arbitrarily choose a "previous" render target
            // that's different
            mPreviousRenderTargetIndex = mRenderTargetIndex =
                mSwapChain->GetCurrentBackBufferIndex();
            mPreviousRenderTargetIndex = mRenderTargetIndex == 0 ? 1 : 0;

            // Initial the serial for all render targets
            const uint64_t initialSerial = backend::d3d12::GetSerial(mBackendDevice);
            for (uint32_t n = 0; n < kFrameCount; ++n) {
                mLastSerialRenderTargetWasUsed[n] = initialSerial;
            }

            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError GetNextTexture(nxtSwapChainNextTexture* nextTexture) {
            nextTexture->texture = mRenderTargetResources[mRenderTargetIndex].Get();
            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError Present() {
            // Current frame already transitioned to Present by the application, but
            // we need to flush the D3D12 backend's pending transitions.
            mProcs.deviceTick(mBackendDevice);

            ASSERT_SUCCESS(mSwapChain->Present(1, 0));

            // Transition last frame's render target back to being a render target
            if (mRenderTargetResourceState != D3D12_RESOURCE_STATE_PRESENT) {
                ComPtr<ID3D12GraphicsCommandList> commandList = {};
                backend::d3d12::OpenCommandList(mBackendDevice, &commandList);

                D3D12_RESOURCE_BARRIER resourceBarrier;
                resourceBarrier.Transition.pResource =
                    mRenderTargetResources[mPreviousRenderTargetIndex].Get();
                resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                resourceBarrier.Transition.StateAfter = mRenderTargetResourceState;
                resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                commandList->ResourceBarrier(1, &resourceBarrier);
                ASSERT_SUCCESS(commandList->Close());
                backend::d3d12::ExecuteCommandLists(mBackendDevice, {commandList.Get()});
            }

            backend::d3d12::NextSerial(mBackendDevice);

            mPreviousRenderTargetIndex = mRenderTargetIndex;
            mRenderTargetIndex = mSwapChain->GetCurrentBackBufferIndex();

            // If the next render target is not ready to be rendered yet, wait until it is ready.
            // If the last completed serial is less than the last requested serial for this render
            // target, then the commands previously executed on this render target have not yet
            // completed
            backend::d3d12::WaitForSerial(mBackendDevice,
                                          mLastSerialRenderTargetWasUsed[mRenderTargetIndex]);

            mLastSerialRenderTargetWasUsed[mRenderTargetIndex] =
                backend::d3d12::GetSerial(mBackendDevice);

            return NXT_SWAP_CHAIN_NO_ERROR;
        }
    };

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

            backend::d3d12::Init(mD3d12Device, procs, device);
            mBackendDevice = *device;
            mProcTable = *procs;
        }

        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                HWND win32Window = glfwGetWin32Window(mWindow);
                mSwapchainImpl = SwapChainImplD3D12::Create(win32Window, mProcTable);
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }

        nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
            return NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
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
