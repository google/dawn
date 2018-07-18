//* Copyright 2017 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.

#ifndef DAWN_WSI_H
#define DAWN_WSI_H

#include <dawn/dawn.h>

// Error message (or nullptr if there was no error)
typedef const char* dawnSwapChainError;
constexpr dawnSwapChainError DAWN_SWAP_CHAIN_NO_ERROR = nullptr;

typedef struct {
    /// Backend-specific texture id/name/pointer
    union {
        void* ptr;
        uint64_t u64;
        uint32_t u32;
    } texture;
} dawnSwapChainNextTexture;

typedef struct {
    /// Initialize the swap chain implementation.
    ///   (*wsiContext) is one of dawnWSIContext{D3D12,Metal,GL}
    void (*Init)(void* userData, void* wsiContext);

    /// Destroy the swap chain implementation.
    void (*Destroy)(void* userData);

    /// Configure/reconfigure the swap chain.
    dawnSwapChainError (*Configure)(void* userData,
                                    dawnTextureFormat format,
                                    dawnTextureUsageBit allowedUsage,
                                    uint32_t width,
                                    uint32_t height);

    /// Acquire the next texture from the swap chain.
    dawnSwapChainError (*GetNextTexture)(void* userData, dawnSwapChainNextTexture* nextTexture);

    /// Present the last acquired texture to the screen.
    dawnSwapChainError (*Present)(void* userData);

    /// Each function is called with userData as its first argument.
    void* userData;

    /// For use by the D3D12 and Vulkan backends: how the swapchain will use the texture.
    dawnTextureUsageBit textureUsage;
} dawnSwapChainImplementation;

#if defined(DAWN_ENABLE_BACKEND_D3D12) && defined(__cplusplus)
typedef struct {
    dawnDevice device = nullptr;
} dawnWSIContextD3D12;
#endif

#if defined(DAWN_ENABLE_BACKEND_METAL) && defined(__OBJC__)
#    import <Metal/Metal.h>

typedef struct {
    id<MTLDevice> device = nil;
} dawnWSIContextMetal;
#endif

#ifdef DAWN_ENABLE_BACKEND_OPENGL
typedef struct {
} dawnWSIContextGL;
#endif

#ifdef DAWN_ENABLE_BACKEND_VULKAN
typedef struct {
} dawnWSIContextVulkan;
#endif

#endif  // DAWN_WSI_H
