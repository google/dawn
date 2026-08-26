// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_EGLFUNCTIONS_H_
#define SRC_DAWN_NATIVE_OPENGL_EGLFUNCTIONS_H_

#include "src/dawn/common/egl_platform.h"
#include "src/dawn/common/ityp_bitset.h"
#include "src/dawn/native/Error.h"

namespace dawn::native::opengl {

enum class EGLExt : uint32_t {
    // Promoted to EGL 1.5
    ClientExtensions,
    PlatformBase,
    CLEvent2,
    WaitSync,
    ImageBase,
    GLTexture2DImage,
    GLTexture3DImage,
    GLTextureCubemapImage,
    GLRenderBufferImage,
    CreateContext,
    CreateContextRobustness,
    GetAllProcAddresses,
    ClientGetAllProcAddresses,
    GLColorSpace,
    SurfacelessContext,

    // Other extensions,
    FenceSync,  // Not marked as promoted due to different function prototypes
    DisplayTextureShareGroup,
    ReusableSync,
    NoConfigContext,
    PixelFormatFloat,
    GLColorspace,
    NativeFenceSync,  // EGL_ANDROID_native_fence_sync

    // EGL image creation extensions
    ImageNativeBuffer,      // EGL_ANDROID_image_native_buffer
    GetNativeClientBuffer,  // EGL_ANDROID_get_native_client_buffer

    // ANGLE specific
    ANGLECreateContextBackwardsCompatible,  // EGL_ANGLE_create_context_backwards_compatible
    ANGLECreateContextExtensionsEnabled,    // EGL_ANGLE_create_context_extensions_enabled
    ANGLEContextVirtualization,             // EGL_ANGLE_context_virtualization
    ANGLECreateContextWebGLCompatibility,   // EGL_ANGLE_create_context_webgl_compatibility

    EnumCount,
};

// An EGL function loader that also takes care of discovering which extensions are available.
// (taking into account the ones that have been promoted to core EGL versions).
class EGLFunctions {
  public:
    MaybeError LoadClientProcs(EGLGetProcProc getProc);
    MaybeError LoadDisplayProcs(EGLDisplay display);

    uint32_t GetMajorVersion() const;
    uint32_t GetMinorVersion() const;
    bool IsAtLeastVersion(uint32_t major, uint32_t minor) const;
    bool HasExt(EGLExt extension) const;

    // EGL 1.0
    PFNEGLGETPROCADDRESSPROC GetProcAddress = nullptr;

    PFNEGLCHOOSECONFIGPROC ChooseConfig = nullptr;
    PFNEGLCOPYBUFFERSPROC CopyBuffers = nullptr;
    PFNEGLCREATECONTEXTPROC CreateContext = nullptr;
    PFNEGLCREATEPBUFFERSURFACEPROC CreatePbufferSurface = nullptr;
    PFNEGLCREATEPIXMAPSURFACEPROC CreatePixmapSurface = nullptr;
    PFNEGLCREATEWINDOWSURFACEPROC CreateWindowSurface = nullptr;
    PFNEGLDESTROYCONTEXTPROC DestroyContext = nullptr;
    PFNEGLDESTROYSURFACEPROC DestroySurface = nullptr;
    PFNEGLGETCONFIGATTRIBPROC GetConfigAttrib = nullptr;
    PFNEGLGETCONFIGSPROC GetConfigs = nullptr;
    PFNEGLGETCURRENTDISPLAYPROC GetCurrentDisplay = nullptr;
    PFNEGLGETCURRENTSURFACEPROC GetCurrentSurface = nullptr;
    PFNEGLGETDISPLAYPROC GetDisplay = nullptr;
    PFNEGLGETERRORPROC GetError = nullptr;
    PFNEGLINITIALIZEPROC Initialize = nullptr;
    PFNEGLMAKECURRENTPROC MakeCurrent = nullptr;
    PFNEGLQUERYCONTEXTPROC QueryContext = nullptr;
    PFNEGLQUERYSTRINGPROC QueryString = nullptr;
    PFNEGLQUERYSURFACEPROC QuerySurface = nullptr;
    PFNEGLSWAPBUFFERSPROC SwapBuffers = nullptr;
    PFNEGLTERMINATEPROC Terminate = nullptr;
    PFNEGLWAITGLPROC WaitGL = nullptr;
    PFNEGLWAITNATIVEPROC WaitNative = nullptr;

    // EGL 1.1
    PFNEGLBINDTEXIMAGEPROC BindTexImage = nullptr;
    PFNEGLRELEASETEXIMAGEPROC ReleaseTexImage = nullptr;
    PFNEGLSURFACEATTRIBPROC SurfaceAttrib = nullptr;
    PFNEGLSWAPINTERVALPROC SwapInterval = nullptr;

    // EGL 1.2
    PFNEGLBINDAPIPROC BindAPI = nullptr;
    PFNEGLQUERYAPIPROC QueryAPI = nullptr;
    PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC CreatePbufferFromClientBuffer = nullptr;
    PFNEGLRELEASETHREADPROC ReleaseThread = nullptr;
    PFNEGLWAITCLIENTPROC WaitClient = nullptr;

    // EGL 1.3 (no new procs)

    // EGL 1.4
    PFNEGLGETCURRENTCONTEXTPROC GetCurrentContext = nullptr;

    // EGL 1.5
    PFNEGLCREATESYNCPROC CreateSync = nullptr;
    PFNEGLDESTROYSYNCPROC DestroySync = nullptr;
    PFNEGLCLIENTWAITSYNCPROC ClientWaitSync = nullptr;
    PFNEGLGETSYNCATTRIBPROC GetSyncAttrib = nullptr;
    PFNEGLCREATEIMAGEPROC CreateImage = nullptr;
    PFNEGLDESTROYIMAGEPROC DestroyImage = nullptr;
    PFNEGLGETPLATFORMDISPLAYPROC GetPlatformDisplay = nullptr;
    PFNEGLCREATEPLATFORMWINDOWSURFACEPROC CreatePlatformWindowSurface = nullptr;
    PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC CreatePlatformPixmapSurface = nullptr;
    PFNEGLWAITSYNCPROC WaitSync = nullptr;

    // EGL_KHR_fence_sync
    // NOTE: These functions use attribute lists with EGLint but the core versions use EGLattrib.
    // They are not compatible.
    PFNEGLCREATESYNCKHRPROC CreateSyncKHR = nullptr;
    PFNEGLDESTROYSYNCKHRPROC DestroySyncKHR = nullptr;
    PFNEGLCLIENTWAITSYNCKHRPROC ClientWaitSyncKHR = nullptr;
    PFNEGLGETSYNCATTRIBKHRPROC GetSyncAttribKHR = nullptr;

    // EGL_KHR_reusable_sync
    PFNEGLSIGNALSYNCKHRPROC SignalSync = nullptr;

    // EGL_ANDROID_get_native_client_buffer
    PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC GetNativeClientBuffer = nullptr;

    // EGL_ANDROID_native_fence_sync
    PFNEGLDUPNATIVEFENCEFDANDROIDPROC DupNativeFenceFD = nullptr;

  private:
    MaybeError LoadClientExtensions();

    uint32_t mMajorVersion = 0;
    uint32_t mMinorVersion = 0;

    ityp::bitset<EGLExt, static_cast<size_t>(EGLExt::EnumCount)> mExtensions;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_EGLFUNCTIONS_H_
