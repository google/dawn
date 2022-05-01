// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_NATIVESWAPCHAINIMPLGL_H_
#define SRC_DAWN_NATIVE_OPENGL_NATIVESWAPCHAINIMPLGL_H_

#include "dawn/native/OpenGLBackend.h"

#include "dawn/native/dawn_platform.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;

class NativeSwapChainImpl {
  public:
    using WSIContext = DawnWSIContextGL;

    NativeSwapChainImpl(Device* device, PresentCallback present, void* presentUserdata);
    ~NativeSwapChainImpl();

    void Init(DawnWSIContextGL* context);
    DawnSwapChainError Configure(WGPUTextureFormat format,
                                 WGPUTextureUsage,
                                 uint32_t width,
                                 uint32_t height);
    DawnSwapChainError GetNextTexture(DawnSwapChainNextTexture* nextTexture);
    DawnSwapChainError Present();

    wgpu::TextureFormat GetPreferredFormat() const;

  private:
    PresentCallback mPresentCallback;
    void* mPresentUserdata;

    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    GLuint mBackFBO = 0;
    GLuint mBackTexture = 0;

    Device* mDevice = nullptr;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_NATIVESWAPCHAINIMPLGL_H_
