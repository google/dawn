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

#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

namespace backend {
namespace metal {
    void Init(id<MTLDevice> metalDevice, nxtProcTable* procs, nxtDevice* device);
    void SetNextDrawable(nxtDevice device, id<CAMetalDrawable> drawable);
    void Present(nxtDevice device);
}
}

namespace utils {

    class MetalBinding : public BackendBinding {
        public:
            void SetupGLFWWindowHints() override {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            }
            void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
                metalDevice = MTLCreateSystemDefaultDevice();

                id nsWindow = glfwGetCocoaWindow(window);
                NSView* contentView = [nsWindow contentView];
                [contentView setWantsLayer: YES];

                layer = [CAMetalLayer layer];
                [layer setDevice: metalDevice];
                [layer setPixelFormat: MTLPixelFormatBGRA8Unorm];
                [layer setFramebufferOnly: YES];
                [layer setDrawableSize: [contentView bounds].size];

                [contentView setLayer: layer];

                backend::metal::Init(metalDevice, procs, device);
                backendDevice = *device;

                backend::metal::SetNextDrawable(backendDevice, GetNextDrawable());
            }
            void SwapBuffers() override {
                backend::metal::Present(backendDevice);
                backend::metal::SetNextDrawable(backendDevice, GetNextDrawable());
            }

        private:
            id<CAMetalDrawable> GetNextDrawable() {
                lastDrawable = [layer nextDrawable];
                return lastDrawable;
            }

            id<MTLDevice> metalDevice = nil;
            CAMetalLayer* layer = nullptr;

            id<CAMetalDrawable> lastDrawable = nil;

            nxtDevice backendDevice = nullptr;
    };

    BackendBinding* CreateMetalBinding() {
        return new MetalBinding;
    }

}
