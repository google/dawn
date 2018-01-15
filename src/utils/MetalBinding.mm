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
#include "common/SwapChainUtils.h"
#include "nxt/nxt_wsi.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace backend { namespace metal {
    void Init(id<MTLDevice> metalDevice, nxtProcTable* procs, nxtDevice* device);
    void SetNextDrawable(nxtDevice device, id<CAMetalDrawable> drawable);
    void Present(nxtDevice device);
}}

namespace utils {
    class SwapChainImplMTL {
      public:
        using WSIContext = nxtWSIContextMetal;

        SwapChainImplMTL(id nsWindow) : mNsWindow(nsWindow) {
        }

        ~SwapChainImplMTL() {
            [mCurrentTexture release];
            [mCurrentDrawable release];
        }

        void Init(nxtWSIContextMetal* ctx) {
            mMtlDevice = ctx->device;
            mCommandQueue = [mMtlDevice newCommandQueue];
        }

        nxtSwapChainError Configure(nxtTextureFormat format,
                                    nxtTextureUsageBit,
                                    uint32_t width,
                                    uint32_t height) {
            if (format != NXT_TEXTURE_FORMAT_B8_G8_R8_A8_UNORM) {
                return "unsupported format";
            }
            ASSERT(width > 0);
            ASSERT(height > 0);

            NSView* contentView = [mNsWindow contentView];
            [contentView setWantsLayer:YES];

            CGSize size = {};
            size.width = width;
            size.height = height;

            mLayer = [CAMetalLayer layer];
            [mLayer setDevice:mMtlDevice];
            [mLayer setPixelFormat:MTLPixelFormatBGRA8Unorm];
            [mLayer setFramebufferOnly:YES];
            [mLayer setDrawableSize:size];

            [contentView setLayer:mLayer];

            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError GetNextTexture(nxtSwapChainNextTexture* nextTexture) {
            [mCurrentDrawable release];
            mCurrentDrawable = [mLayer nextDrawable];
            [mCurrentDrawable retain];

            [mCurrentTexture release];
            mCurrentTexture = mCurrentDrawable.texture;
            [mCurrentTexture retain];

            nextTexture->texture = reinterpret_cast<void*>(mCurrentTexture);

            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError Present() {
            id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];
            [commandBuffer presentDrawable:mCurrentDrawable];
            [commandBuffer commit];

            return NXT_SWAP_CHAIN_NO_ERROR;
        }

      private:
        id mNsWindow = nil;
        id<MTLDevice> mMtlDevice = nil;
        id<MTLCommandQueue> mCommandQueue = nil;

        CAMetalLayer* mLayer = nullptr;
        id<CAMetalDrawable> mCurrentDrawable = nil;
        id<MTLTexture> mCurrentTexture = nil;
    };

    class MetalBinding : public BackendBinding {
      public:
        void SetupGLFWWindowHints() override {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            mMetalDevice = MTLCreateSystemDefaultDevice();

            backend::metal::Init(mMetalDevice, procs, device);
            mBackendDevice = *device;
        }

        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                mSwapchainImpl = CreateSwapChainImplementation(
                    new SwapChainImplMTL(glfwGetCocoaWindow(mWindow)));
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }

        nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
            return NXT_TEXTURE_FORMAT_B8_G8_R8_A8_UNORM;
        }

      private:
        id<MTLDevice> mMetalDevice = nil;
        nxtDevice mBackendDevice = nullptr;
        nxtSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateMetalBinding() {
        return new MetalBinding;
    }
}
