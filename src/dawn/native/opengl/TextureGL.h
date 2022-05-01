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

#ifndef SRC_DAWN_NATIVE_OPENGL_TEXTUREGL_H_
#define SRC_DAWN_NATIVE_OPENGL_TEXTUREGL_H_

#include "dawn/native/Texture.h"

#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;
struct GLFormat;

class Texture final : public TextureBase {
  public:
    Texture(Device* device, const TextureDescriptor* descriptor);
    Texture(Device* device, const TextureDescriptor* descriptor, GLuint handle, TextureState state);

    GLuint GetHandle() const;
    GLenum GetGLTarget() const;
    const GLFormat& GetGLFormat() const;
    uint32_t GetGenID() const;
    void Touch();

    void EnsureSubresourceContentInitialized(const SubresourceRange& range);

  private:
    ~Texture() override;

    void DestroyImpl() override;
    MaybeError ClearTexture(const SubresourceRange& range, TextureBase::ClearValue clearValue);

    GLuint mHandle;
    GLenum mTarget;
    uint32_t mGenID = 0;
};

class TextureView final : public TextureViewBase {
  public:
    TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor);

    GLuint GetHandle() const;
    GLenum GetGLTarget() const;
    void BindToFramebuffer(GLenum target, GLenum attachment);
    void CopyIfNeeded();

  private:
    ~TextureView() override;
    void DestroyImpl() override;
    GLenum GetInternalFormat() const;

    // TODO(crbug.com/dawn/1355): Delete this handle on texture destroy.
    GLuint mHandle;
    GLenum mTarget;
    bool mOwnsHandle;
    bool mUseCopy = false;
    uint32_t mGenID = 0;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_TEXTUREGL_H_
