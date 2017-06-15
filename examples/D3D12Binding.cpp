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

#include "BackendBinding.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <initializer_list>
#include <assert.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#define ASSERT assert

using Microsoft::WRL::ComPtr;

namespace backend {
namespace d3d12 {
    void Init(ComPtr<ID3D12Device> d3d12Device, nxtProcTable* procs, nxtDevice* device);
    ComPtr<ID3D12CommandQueue> GetCommandQueue(nxtDevice device);
    void SetNextRenderTargetDescriptor(nxtDevice device, D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDescriptor);
    uint64_t GetSerial(const nxtDevice device);
    void NextSerial(nxtDevice device);
    void ExecuteCommandLists(nxtDevice device, std::initializer_list<ID3D12CommandList*> commandLists);
    void WaitForSerial(nxtDevice device, uint64_t serial);
    ComPtr<ID3D12CommandAllocator> ReserveCommandAllocator(nxtDevice device);
}
}

class D3D12Binding : public BackendBinding {
    public:
        void SetupGLFWWindowHints() override {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            uint32_t dxgiFactoryFlags = 0;
#ifdef _DEBUG
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            // NOTE: Enabling the debug layer after device creation will invalidate the active device.
            {
                ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                {
                    debugController->EnableDebugLayer();

                    // Enable additional debug layers.
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }
            }
#endif

            ASSERT_SUCCESS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
            ASSERT(GetHardwareAdapter(factory.Get(), &hardwareAdapter));
            ASSERT_SUCCESS(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&d3d12Device)
            ));

            backend::d3d12::Init(d3d12Device, procs, device);
            backendDevice = *device;
            commandQueue = backend::d3d12::GetCommandQueue(backendDevice);

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = kFrameCount;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;

            HWND win32Window = glfwGetWin32Window(window);
            ComPtr<IDXGISwapChain1> swapChain1;
            ASSERT_SUCCESS(factory->CreateSwapChainForHwnd(
                commandQueue.Get(),
                win32Window,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain1
            ));
            ASSERT_SUCCESS(swapChain1.As(&swapChain));

            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = kFrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ASSERT_SUCCESS(d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&renderTargetViewHeap)));

            rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            // Create a RTV for each frame.
            {
                D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
                for (uint32_t n = 0; n < kFrameCount; ++n) {
                    ASSERT_SUCCESS(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargetResources[n])));
                    d3d12Device->CreateRenderTargetView(renderTargetResources[n].Get(), nullptr, renderTargetViewHandle);
                    renderTargetViewHandle.ptr += rtvDescriptorSize;
                }
            }

            // Get the initial render target and arbitrarily choose a "previous" render target that's different
            previousRenderTargetIndex = renderTargetIndex = swapChain->GetCurrentBackBufferIndex();
            previousRenderTargetIndex = renderTargetIndex == 0 ? 1 : 0;

            // Initial the serial for all render targets
            const uint64_t initialSerial = backend::d3d12::GetSerial(backendDevice);
            for (uint32_t n = 0; n < kFrameCount; ++n) {
                lastSerialRenderTargetWasUsed[n] = initialSerial;
            }

            // Transition the first frame to be a render target
            {
                ComPtr<ID3D12CommandAllocator> commandAllocator = backend::d3d12::ReserveCommandAllocator(backendDevice);
                ASSERT_SUCCESS(d3d12Device->CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    commandAllocator.Get(),
                    nullptr,
                    IID_PPV_ARGS(&commandList)
                ));

                D3D12_RESOURCE_BARRIER resourceBarrier;
                resourceBarrier.Transition.pResource = renderTargetResources[renderTargetIndex].Get();
                resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                commandList->ResourceBarrier(1, &resourceBarrier);
                ASSERT_SUCCESS(commandList->Close());
                ID3D12CommandList* commandLists[] = { commandList.Get() };
                backend::d3d12::ExecuteCommandLists(backendDevice, { commandList.Get() });

                backend::d3d12::NextSerial(backendDevice);
            }

            D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
            renderTargetViewHandle.ptr += rtvDescriptorSize * renderTargetIndex;
            backend::d3d12::SetNextRenderTargetDescriptor(backendDevice, renderTargetViewHandle);
        }

        void SwapBuffers() override {
            // Transition current frame's render target for presenting
            {
                ComPtr<ID3D12CommandAllocator> commandAllocator = backend::d3d12::ReserveCommandAllocator(backendDevice);
                ASSERT_SUCCESS(commandList->Reset(commandAllocator.Get(), nullptr));
                D3D12_RESOURCE_BARRIER resourceBarrier;
                resourceBarrier.Transition.pResource = renderTargetResources[renderTargetIndex].Get();
                resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                commandList->ResourceBarrier(1, &resourceBarrier);
                ASSERT_SUCCESS(commandList->Close());
                ID3D12CommandList* commandLists[] = { commandList.Get() };
                backend::d3d12::ExecuteCommandLists(backendDevice, { commandList.Get() });
            }

            ASSERT_SUCCESS(swapChain->Present(1, 0));

            // Transition last frame's render target back to being a render target
            {
                ComPtr<ID3D12CommandAllocator> commandAllocator = backend::d3d12::ReserveCommandAllocator(backendDevice);
                ASSERT_SUCCESS(commandList->Reset(commandAllocator.Get(), nullptr));
                D3D12_RESOURCE_BARRIER resourceBarrier;
                resourceBarrier.Transition.pResource = renderTargetResources[previousRenderTargetIndex].Get();
                resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                commandList->ResourceBarrier(1, &resourceBarrier);
                ASSERT_SUCCESS(commandList->Close());
                ID3D12CommandList* commandLists[] = { commandList.Get() };
                backend::d3d12::ExecuteCommandLists(backendDevice, { commandList.Get() });
            }

            backend::d3d12::NextSerial(backendDevice);

            previousRenderTargetIndex = renderTargetIndex;
            renderTargetIndex = swapChain->GetCurrentBackBufferIndex();

            // If the next render target is not ready to be rendered yet, wait until it is ready.
            // If the last completed serial is less than the last requested serial for this render target,
            // then the commands previously executed on this render target have not yet completed
            backend::d3d12::WaitForSerial(backendDevice, lastSerialRenderTargetWasUsed[renderTargetIndex]);

            lastSerialRenderTargetWasUsed[renderTargetIndex] = backend::d3d12::GetSerial(backendDevice);

            // Tell the backend to render to the current render target
            D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
            renderTargetViewHandle.ptr += rtvDescriptorSize * renderTargetIndex;
            backend::d3d12::SetNextRenderTargetDescriptor(backendDevice, renderTargetViewHandle);
        }

    private:
        nxtDevice backendDevice = nullptr;

        static constexpr unsigned int kFrameCount = 2;

        // Initialization
        ComPtr<IDXGIFactory4> factory;
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        ComPtr<ID3D12Device> d3d12Device;
        ComPtr<ID3D12CommandQueue> commandQueue;
        ComPtr<IDXGISwapChain3> swapChain;
        ComPtr<ID3D12DescriptorHeap> renderTargetViewHeap;
        ComPtr<ID3D12Resource> renderTargetResources[kFrameCount];
        uint32_t rtvDescriptorSize;

        // Frame synchronization. Updated every frame
        uint32_t renderTargetIndex;
        uint32_t previousRenderTargetIndex;
        uint64_t lastSerialRenderTargetWasUsed[kFrameCount];
        ComPtr<ID3D12GraphicsCommandList> commandList;

        static void ASSERT_SUCCESS(HRESULT hr) {
            assert(SUCCEEDED(hr));
        }

        static bool GetHardwareAdapter(IDXGIFactory4* factory, IDXGIAdapter1** hardwareAdapter) {
            *hardwareAdapter = nullptr;
            for (uint32_t adapterIndex = 0; ; ++adapterIndex) {
                IDXGIAdapter1* adapter = nullptr;
                if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND) {
                    break; // No more adapters to enumerate.
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
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
