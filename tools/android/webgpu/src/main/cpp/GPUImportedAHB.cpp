// Copyright 2026 The Dawn & Tint Authors
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

#include <android/hardware_buffer.h>
#include <dlfcn.h>
#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <vector>

#include "JNIClasses.h"
#include "JNIContext.h"
#include "src/dawn/common/ColorSpace.h"
#include "src/dawn/common/ExternalTextureParams.h"
#include "structures.h"

#ifndef VK_IMAGE_LAYOUT_GENERAL
#define VK_IMAGE_LAYOUT_GENERAL 1
#endif

#ifndef VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
#define VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 5
#endif

namespace dawn::kotlin_api {

struct AhbTextureWrapper {
    wgpu::SharedTextureMemory stm;
    wgpu::Texture texture;
    wgpu::TextureView view;
    wgpu::ExternalTexture externalTexture;
    AHardwareBuffer* ahb = nullptr;
    wgpu::Device device;
};

struct UniqueFd {
    int fd = -1;
    UniqueFd(int f) : fd(f) {}
    ~UniqueFd() {
        if (fd >= 0) {
            close(fd);
        }
    }
    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;
    UniqueFd(UniqueFd&& other) : fd(other.fd) { other.fd = -1; }
    UniqueFd& operator=(UniqueFd&& other) {
        if (this != &other) {
            if (fd >= 0) {
                close(fd);
            }
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    int release() {
        int f = fd;
        fd = -1;
        return f;
    }
    operator int() const { return fd; }
};

namespace {

using PFN_FromHB = AHardwareBuffer* (*)(JNIEnv*, jobject);
using PFN_Acquire = void (*)(AHardwareBuffer*);
using PFN_Release = void (*)(AHardwareBuffer*);
using PFN_SyncMerge = int32_t (*)(const char*, int32_t, int32_t);

struct AndroidApi26Symbols {
    PFN_FromHB fromHardwareBuffer = nullptr;
    PFN_Acquire acquire = nullptr;
    PFN_Release release = nullptr;
    PFN_SyncMerge syncMerge = nullptr;
};

// Dynamically load API 26 symbols rather than linking directly: minSdk is 24,
// AHardwareBuffer_* and sync_merge require API 26, and libsync.so is not in
// the public linker namespace before API 26 (so libsync.so must stay dlopen
// even if AHB symbols move to weak references). The dlopen handles are
// intentionally never closed so the resolved pointers stay valid.
const AndroidApi26Symbols& GetAndroidApi26Symbols() {
    static const AndroidApi26Symbols kSymbols = [] {
        AndroidApi26Symbols sym;
        if (void* lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL)) {
            void* fromSym = dlsym(lib, "AHardwareBuffer_fromHardwareBuffer");
            void* acqSym = dlsym(lib, "AHardwareBuffer_acquire");
            void* relSym = dlsym(lib, "AHardwareBuffer_release");
            sym.fromHardwareBuffer = reinterpret_cast<PFN_FromHB>(fromSym);
            sym.acquire = reinterpret_cast<PFN_Acquire>(acqSym);
            sym.release = reinterpret_cast<PFN_Release>(relSym);
        }
        if (void* lib = dlopen("libsync.so", RTLD_NOW | RTLD_LOCAL)) {
            void* mergeSym = dlsym(lib, "sync_merge");
            sym.syncMerge = reinterpret_cast<PFN_SyncMerge>(mergeSym);
        }
        return sym;
    }();
    return kSymbols;
}

void ReleaseAHardwareBuffer(AHardwareBuffer* ahb) {
    if (ahb != nullptr) {
        if (auto release = GetAndroidApi26Symbols().release) {
            release(ahb);
        }
    }
}

}  // namespace

extern "C" {

static wgpu::SharedTextureMemory createSharedTextureMemoryFromAhb(JNIEnv* env,
                                                                  jobject deviceObj,
                                                                  jobject hardwareBuffer,
                                                                  AHardwareBuffer** outAhb,
                                                                  wgpu::Device* outDevice) {
    JNIClasses* classes = JNIClasses::getInstance(env);
    if (!deviceObj || !hardwareBuffer) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "Device and HardwareBuffer must not be null.");
        return nullptr;
    }

    jmethodID getHandle = env->GetMethodID(classes->device, "getHandle", "()J");
    if (!getHandle || env->ExceptionCheck()) {
        return nullptr;
    }

    WGPUDevice rawDevice = reinterpret_cast<WGPUDevice>(env->CallLongMethod(deviceObj, getHandle));
    if (env->ExceptionCheck()) {
        return nullptr;
    }
    if (!rawDevice) {
        env->ThrowNew(classes->javaIllegalStateException, "WGPUDevice handle is null.");
        return nullptr;
    }
    *outDevice = wgpu::Device(rawDevice);

    const auto& api26 = GetAndroidApi26Symbols();
    if (!api26.fromHardwareBuffer || !api26.acquire || !api26.release) {
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "AHardwareBuffer requires Android API 26 or higher.");
        return nullptr;
    }

    *outAhb = api26.fromHardwareBuffer(env, hardwareBuffer);
    if (!*outAhb) {
        if (!env->ExceptionCheck()) {
            env->ThrowNew(classes->javaIllegalStateException,
                          "Failed to extract native AHardwareBuffer.");
        }
        return nullptr;
    }
    // AHardwareBuffer_fromHardwareBuffer does not acquire a reference; the returned pointer
    // is only guaranteed to be valid while the Java HardwareBuffer remains alive. Since the
    // wrapper outlives the JNI call, take our own reference to match the release in
    // nativeDestroy.
    api26.acquire(*outAhb);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor ahbDesc = {};
    ahbDesc.handle = *outAhb;

    wgpu::SharedTextureMemoryDescriptor stmDesc = {};
    stmDesc.nextInChain = &ahbDesc;

    wgpu::SharedTextureMemory stm = outDevice->ImportSharedTextureMemory(&stmDesc);
    if (!stm && !env->ExceptionCheck()) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "Dawn failed to import the AHardwareBuffer.");
    }
    return stm;
}

JNIEXPORT jobject JNICALL
Java_androidx_webgpu_helper_GPUAndroidHardwareBufferUtil_createTextureNative(JNIEnv* env,
                                                                             jclass clazz,
                                                                             jobject deviceObj,
                                                                             jobject hardwareBuffer,
                                                                             jint usage) {
    JNIContext c(env);
    JNIClasses* classes = JNIClasses::getInstance(env);
    wgpu::Device device = nullptr;
    AHardwareBuffer* ahb = nullptr;

    wgpu::SharedTextureMemory stm =
        createSharedTextureMemoryFromAhb(env, deviceObj, hardwareBuffer, &ahb, &device);
    if (!stm || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    if (!device.HasFeature(wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer)) {
        ReleaseAHardwareBuffer(ahb);
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "Missing wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer.");
        return nullptr;
    }

    wgpu::SharedTextureMemoryProperties props = {};
    stm.GetProperties(&props);
    if (env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    if (props.format == wgpu::TextureFormat::Undefined) {
        ReleaseAHardwareBuffer(ahb);
        env->ThrowNew(
            classes->javaUnsupportedOperationException,
            "AHB format maps to wgpu::TextureFormat::Undefined. Dawn cannot sample this.");
        return nullptr;
    }

    wgpu::TextureDescriptor texDesc = {};
    texDesc.usage = static_cast<wgpu::TextureUsage>(usage);
    texDesc.dimension = wgpu::TextureDimension::e2D;
    uint32_t depth = props.size.depthOrArrayLayers == 0 ? 1 : props.size.depthOrArrayLayers;
    texDesc.size = {props.size.width, props.size.height, depth};
    texDesc.format = props.format;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;

    wgpu::Texture texture = stm.CreateTexture(&texDesc);
    if (!texture || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    jclass texClz = classes->texture;
    jmethodID texInit = env->GetMethodID(texClz, "<init>", "(J)V");
    if (!texInit || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }
    jclass wrapClz = classes->gpuHardwareBufferTexture;
    const char* wrapSig = "(JLandroidx/webgpu/GPUTexture;)V";
    jmethodID wrapInit = env->GetMethodID(wrapClz, "<init>", wrapSig);
    if (!wrapInit || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    // Both GPUTexture and AhbTextureWrapper independently release a reference
    // on close(). Add GPUTexture's reference before constructing texture_kt so
    // texture_kt never holds an unowned handle.
    wgpuTextureAddRef(texture.Get());
    jlong texPtr = reinterpret_cast<jlong>(texture.Get());
    jobject texture_kt = env->NewObject(texClz, texInit, texPtr);
    if (!texture_kt || env->ExceptionCheck()) {
        wgpuTextureRelease(texture.Get());
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    auto wrapper = std::make_unique<AhbTextureWrapper>();
    wrapper->stm = std::move(stm);
    wrapper->texture = std::move(texture);
    wrapper->ahb = ahb;
    wrapper->device = std::move(device);

    jlong wrapPtr = reinterpret_cast<jlong>(wrapper.get());
    jobject wrapperObj = env->NewObject(wrapClz, wrapInit, wrapPtr, texture_kt);
    if (!wrapperObj || env->ExceptionCheck()) {
        env->DeleteLocalRef(texture_kt);
        wgpuTextureRelease(wrapper->texture.Get());
        ReleaseAHardwareBuffer(wrapper->ahb);
        return nullptr;
    }

    wrapper.release();
    return wrapperObj;
}

JNIEXPORT jobject JNICALL
Java_androidx_webgpu_helper_GPUAndroidHardwareBufferUtil_createExternalTextureNative(
    JNIEnv* env,
    jclass clazz,
    jobject deviceObj,
    jobject hardwareBuffer,
    jobject descriptorObj) {
    JNIContext c(env);
    JNIClasses* classes = JNIClasses::getInstance(env);
    if (!descriptorObj) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "GPUExternalTextureDescriptor must not be null.");
        return nullptr;
    }

    wgpu::Device device = nullptr;
    AHardwareBuffer* ahb = nullptr;

    wgpu::SharedTextureMemory stm =
        createSharedTextureMemoryFromAhb(env, deviceObj, hardwareBuffer, &ahb, &device);
    if (!stm || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    if (!device.HasFeature(wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture)) {
        ReleaseAHardwareBuffer(ahb);
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "Missing wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture.");
        return nullptr;
    }

    wgpu::SharedTextureMemoryProperties props = {};
    stm.GetProperties(&props);
    if (env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    if (props.format == wgpu::TextureFormat::Undefined) {
        ReleaseAHardwareBuffer(ahb);
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "YUV AHB format maps to wgpu::TextureFormat::Undefined.");
        return nullptr;
    }

    wgpu::TextureDescriptor texDesc = {};
    texDesc.usage = wgpu::TextureUsage::TextureBinding;
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.size = {props.size.width, props.size.height, 1};
    texDesc.format = props.format;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;

    wgpu::Texture texture = stm.CreateTexture(&texDesc);
    if (!texture || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    wgpu::TextureView view = texture.CreateView();
    if (!view || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    wgpu::ExternalTextureDescriptor extDesc = {};
    extDesc.plane0 = view;

    jclass descClass = env->GetObjectClass(descriptorObj);
    if (!descClass || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    auto callObj = [&](const char* name, const char* sig) -> jobject {
        if (env->ExceptionCheck()) {
            return nullptr;
        }
        jmethodID mid = env->GetMethodID(descClass, name, sig);
        if (!mid || env->ExceptionCheck()) {
            return nullptr;
        }
        return env->CallObjectMethod(descriptorObj, mid);
    };

    const char* csSig = "()Landroidx/webgpu/GPUColorSpaceDawn;";
    jobject colorSpaceObj = callObj("getSrcColorSpace", csSig);
    if (!colorSpaceObj || env->ExceptionCheck()) {
        env->DeleteLocalRef(descClass);
        ReleaseAHardwareBuffer(ahb);
        if (!env->ExceptionCheck()) {
            const char* msg = "Missing ExternalTexture srcColorSpace.";
            env->ThrowNew(classes->javaIllegalStateException, msg);
        }
        return nullptr;
    }

    jmethodID dstCsMid = env->GetMethodID(descClass, "getDstColorSpace", "()I");
    if (!dstCsMid || env->ExceptionCheck()) {
        env->DeleteLocalRef(colorSpaceObj);
        env->DeleteLocalRef(descClass);
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }
    jint dstColorSpaceInt = env->CallIntMethod(descriptorObj, dstCsMid);
    if (env->ExceptionCheck()) {
        env->DeleteLocalRef(colorSpaceObj);
        env->DeleteLocalRef(descClass);
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    wgpu::ColorSpaceDawn srcColorSpace = {};
    srcColorSpace.primaries = static_cast<wgpu::ColorSpacePrimariesDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpacePrimaries));
    srcColorSpace.transfer = static_cast<wgpu::ColorSpaceTransferDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpaceTransfer));
    srcColorSpace.yCbCrRange = static_cast<wgpu::ColorSpaceYCbCrRangeDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpaceYCbCrRange));
    srcColorSpace.yCbCrMatrix = static_cast<wgpu::ColorSpaceYCbCrMatrixDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpaceYCbCrMatrix));
    env->DeleteLocalRef(colorSpaceObj);

    wgpu::PredefinedColorSpace dstColorSpace =
        static_cast<wgpu::PredefinedColorSpace>(dstColorSpaceInt);

    dawn::ExternalTextureColorSpaceParams params;
    wgpu::Status status = dawn::ComputeExternalTextureParams(srcColorSpace, dstColorSpace, &params);

    if (status != wgpu::Status::Success) {
        env->DeleteLocalRef(descClass);
        ReleaseAHardwareBuffer(ahb);
        env->ThrowNew(classes->javaIllegalStateException,
                      "Failed to compute external texture params.");
        return nullptr;
    }

    extDesc.yuvToRgbConversionMatrix = params.yuvToRgbConversionMatrix.data();
    extDesc.srcTransferFunctionParameters = params.srcTransferFunction.data();
    extDesc.dstTransferFunctionParameters = params.dstTransferFunction.data();
    extDesc.gamutConversionMatrix = params.gamutConversionMatrix.data();

    const char* origSig = "()Landroidx/webgpu/GPUOrigin2D;";
    jobject originObj = callObj("getCropOrigin", origSig);
    if (originObj) {
        extDesc.cropOrigin.x = env->GetIntField(originObj, classes->originX);
        extDesc.cropOrigin.y = env->GetIntField(originObj, classes->originY);
        env->DeleteLocalRef(originObj);
    }

    const char* extSig = "()Landroidx/webgpu/GPUExtent2D;";
    jobject cropSizeObj = callObj("getCropSize", extSig);
    if (cropSizeObj) {
        extDesc.cropSize.width = env->GetIntField(cropSizeObj, classes->extentWidth);
        extDesc.cropSize.height = env->GetIntField(cropSizeObj, classes->extentHeight);
        env->DeleteLocalRef(cropSizeObj);
    }
    if (extDesc.cropSize.width == 0) {
        extDesc.cropSize.width = props.size.width;
    }
    if (extDesc.cropSize.height == 0) {
        extDesc.cropSize.height = props.size.height;
    }

    extDesc.apparentSize = extDesc.cropSize;
    jmethodID mirrorMid = nullptr;
    if (!env->ExceptionCheck()) {
        mirrorMid = env->GetMethodID(descClass, "isMirrored", "()Z");
    }
    if (mirrorMid && !env->ExceptionCheck()) {
        jboolean mirrored = env->CallBooleanMethod(descriptorObj, mirrorMid);
        extDesc.mirrored = static_cast<bool>(mirrored);
    }

    jmethodID rotMid = nullptr;
    if (!env->ExceptionCheck()) {
        rotMid = env->GetMethodID(descClass, "getRotation", "()I");
    }
    jint rotation = 0;
    if (rotMid && !env->ExceptionCheck()) {
        rotation = env->CallIntMethod(descriptorObj, rotMid);
    }
    env->DeleteLocalRef(descClass);
    if (!mirrorMid || !rotMid || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    switch (rotation) {
        case 2:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate90Degrees;
            break;
        case 3:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate180Degrees;
            break;
        case 4:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate270Degrees;
            break;
        default:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate0Degrees;
            break;
    }

    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&extDesc);
    if (!externalTexture || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    jclass extClz = classes->externalTexture;
    jmethodID extInit = env->GetMethodID(extClz, "<init>", "(J)V");
    if (!extInit || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }
    jclass wrapClz = classes->gpuHardwareBufferExternalTexture;
    const char* wrapSig = "(JLandroidx/webgpu/GPUExternalTexture;)V";
    jmethodID wrapInit = env->GetMethodID(wrapClz, "<init>", wrapSig);
    if (!wrapInit || env->ExceptionCheck()) {
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    // Both GPUExternalTexture and AhbTextureWrapper independently release a
    // reference on close(). Add GPUExternalTexture's reference before
    // constructing extTex_kt so extTex_kt never holds an unowned handle.
    wgpuExternalTextureAddRef(externalTexture.Get());
    jlong extPtr = reinterpret_cast<jlong>(externalTexture.Get());
    jobject extTex_kt = env->NewObject(extClz, extInit, extPtr);
    if (!extTex_kt || env->ExceptionCheck()) {
        wgpuExternalTextureRelease(externalTexture.Get());
        ReleaseAHardwareBuffer(ahb);
        return nullptr;
    }

    auto wrapper = std::make_unique<AhbTextureWrapper>();
    wrapper->stm = std::move(stm);
    wrapper->texture = std::move(texture);
    wrapper->view = std::move(view);
    wrapper->externalTexture = std::move(externalTexture);
    wrapper->ahb = ahb;
    wrapper->device = std::move(device);

    jlong wrapPtr = reinterpret_cast<jlong>(wrapper.get());
    jobject wrapperObj = env->NewObject(wrapClz, wrapInit, wrapPtr, extTex_kt);
    if (!wrapperObj || env->ExceptionCheck()) {
        env->DeleteLocalRef(extTex_kt);
        wgpuExternalTextureRelease(wrapper->externalTexture.Get());
        ReleaseAHardwareBuffer(wrapper->ahb);
        return nullptr;
    }

    wrapper.release();
    return wrapperObj;
}

JNIEXPORT void JNICALL
Java_androidx_webgpu_GPUHardwareBufferWrapper_nativeBeginAccess(JNIEnv* env,
                                                                jobject thiz,
                                                                jlong handle,
                                                                jintArray fences) {
    auto* wrapper = reinterpret_cast<AhbTextureWrapper*>(handle);
    JNIClasses* classes = JNIClasses::getInstance(env);
    jclass illegalStateClz = classes->javaIllegalStateException;
    if (!wrapper) {
        env->ThrowNew(illegalStateClz, "Null native AHB wrapper.");
        return;
    }

    std::vector<wgpu::SharedFence> importedFences;
    if (fences != nullptr) {
        jsize len = env->GetArrayLength(fences);
        if (len > 0) {
            jint* fds = env->GetIntArrayElements(fences, nullptr);
            if (!fds) {
                return;
            }
            for (int i = 0; i < len; ++i) {
                int fd = fds[i];
                if (fd >= 0) {
                    // We only borrow the integer from Java's ParcelFileDescriptor.getFd().
                    // Dawn's ImportSharedFence internally duplicates (dup) the file descriptor.
                    // This allows Java's ParcelFileDescriptor to retain ownership of the original
                    // FD and close it safely when it gets garbage collected, preventing leaks.
                    wgpu::SharedFenceSyncFDDescriptor syncFdDesc = {};
                    syncFdDesc.handle = fd;

                    wgpu::SharedFenceDescriptor fenceDesc = {};
                    fenceDesc.nextInChain = &syncFdDesc;

                    wgpu::SharedFence fence = wrapper->device.ImportSharedFence(&fenceDesc);
                    if (env->ExceptionCheck()) {
                        env->ReleaseIntArrayElements(fences, fds, JNI_ABORT);
                        return;
                    }
                    if (fence) {
                        importedFences.push_back(fence);
                    }
                }
            }
            env->ReleaseIntArrayElements(fences, fds, JNI_ABORT);
        }
    }

    std::vector<uint64_t> signaledValues(importedFences.size(), 1);
    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.fenceCount = importedFences.size();
    beginDesc.fences = importedFences.data();
    beginDesc.signaledValueCount = signaledValues.size();
    beginDesc.signaledValues = signaledValues.data();
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;

    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout = {};
    wgpu::AdapterInfo adapterInfo;
    wrapper->device.GetAdapterInfo(&adapterInfo);
    if (env->ExceptionCheck()) {
        return;
    }
    if (adapterInfo.backendType == wgpu::BackendType::Vulkan) {
        beginLayout.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        beginLayout.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        beginDesc.nextInChain = &beginLayout;
    }

    auto status = wrapper->stm.BeginAccess(wrapper->texture, &beginDesc);
    if (status != wgpu::Status::Success && !env->ExceptionCheck()) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "Failed to begin access on SharedTextureMemory.");
    }
}

JNIEXPORT jint JNICALL Java_androidx_webgpu_GPUHardwareBufferWrapper_nativeEndAccess(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong handle) {
    auto* wrapper = reinterpret_cast<AhbTextureWrapper*>(handle);
    JNIClasses* classes = JNIClasses::getInstance(env);
    jclass illegalStateClz = classes->javaIllegalStateException;
    if (!wrapper) {
        env->ThrowNew(illegalStateClz, "Null native AHB wrapper.");
        return -1;
    }

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    wgpu::SharedTextureMemoryVkImageLayoutEndState endLayout = {};
    wgpu::AdapterInfo adapterInfo;
    wrapper->device.GetAdapterInfo(&adapterInfo);
    if (env->ExceptionCheck()) {
        return -1;
    }
    if (adapterInfo.backendType == wgpu::BackendType::Vulkan) {
        endLayout.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        endLayout.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        endState.nextInChain = &endLayout;
    }

    wgpu::Status status = wrapper->stm.EndAccess(wrapper->texture, &endState);
    if (status != wgpu::Status::Success || env->ExceptionCheck()) {
        if (!env->ExceptionCheck()) {
            env->ThrowNew(classes->javaIllegalStateException,
                          "Failed to end access on SharedTextureMemory.");
        }
        return -1;
    }

    std::vector<UniqueFd> fds;
    bool failed = false;
    if (endState.fenceCount > 0) {
        for (size_t i = 0; i < endState.fenceCount && !failed; ++i) {
            wgpu::SharedFence cFence = endState.fences[i];

            wgpu::SharedFenceExportInfo exportInfo = {};
            wgpu::SharedFenceSyncFDExportInfo syncFdExportInfo = {};
            exportInfo.nextInChain = &syncFdExportInfo;

            cFence.ExportInfo(&exportInfo);
            if (env->ExceptionCheck()) {
                failed = true;
                break;
            }
            int fd = syncFdExportInfo.handle;
            if (fd >= 0) {
                // Dawn's ExportInfo borrows the FD; Dawn retains ownership of the underlying fence
                // and closes it when the WGPUSharedFence is released. We dup() it here to safely
                // take ownership of our own copy for JNI.
                int dupedFd = dup(fd);
                if (dupedFd >= 0) {
                    fds.emplace_back(dupedFd);
                } else {
                    failed = true;
                }
            }
        }
    }

    UniqueFd mergedFd(-1);
    const auto& api26 = GetAndroidApi26Symbols();
    for (size_t i = 0; i < fds.size() && !failed; ++i) {
        UniqueFd& fd = fds[i];
        if (mergedFd.fd == -1) {
            mergedFd = std::move(fd);
        } else if (!api26.syncMerge) {
            failed = true;
        } else {
            int newFd = api26.syncMerge("wgpu_fence", mergedFd.fd, fd.fd);
            mergedFd = UniqueFd(newFd);
            if (newFd < 0) {
                failed = true;
            }
        }
    }

    if (failed || env->ExceptionCheck()) {
        if (!env->ExceptionCheck()) {
            env->ThrowNew(classes->javaIllegalStateException, "Failed to merge sync fences.");
        }
        return -1;
    }

    if (mergedFd.fd >= 0) {
        return mergedFd.release();
    }

    return -1;
}

JNIEXPORT void JNICALL Java_androidx_webgpu_GPUHardwareBufferWrapper_nativeDestroy(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong handle) {
    auto* wrapper = reinterpret_cast<AhbTextureWrapper*>(handle);
    if (!wrapper) {
        return;
    }

    if (wrapper->ahb) {
        ReleaseAHardwareBuffer(wrapper->ahb);
    }

    delete wrapper;
}

JNIEXPORT jboolean JNICALL
Java_androidx_webgpu_GPUSyncFence_awaitParcelFileDescriptorNative(JNIEnv* env,
                                                                  jobject thiz,
                                                                  jobject pfd,
                                                                  jlong timeoutMillis) {
    if (!pfd) {
        return JNI_TRUE;
    }

    JNIClasses* classes = JNIClasses::getInstance(env);
    jmethodID getFdMethod = classes->pfdGetFd;
    int fd = env->CallIntMethod(pfd, getFdMethod);
    if (env->ExceptionCheck()) {
        return JNI_FALSE;
    }

    if (fd < 0) {
        return JNI_TRUE;
    }

    struct pollfd pfd_struct = {
        .fd = fd,
        .events = POLLIN,
    };

    int result = poll(&pfd_struct, 1, timeoutMillis);
    return (result > 0 && (pfd_struct.revents & POLLIN)) ? JNI_TRUE : JNI_FALSE;
}

}  // extern "C"
}  // namespace dawn::kotlin_api
