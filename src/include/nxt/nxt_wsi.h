//* Copyright 2017 The NXT Authors
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

#ifndef NXT_WSI_H
#define NXT_WSI_H

#include <nxt/nxt.h>

// Error message (or nullptr if there was no error)
typedef const char* nxtSwapChainError;

typedef struct {
    /// Backend-specific texture id/name/pointer
    void* texture = nullptr;
} nxtSwapChainNextTexture;

typedef struct {
    /// Initialize the swap chain implementation.
    ///   (*wsiContext) is one of nxtWSIContext{D3D12,Metal,GL}
    void (*Init)(void* userData, void* wsiContext);

    /// Destroy the swap chain implementation.
    void (*Destroy)(void* userData);

    /// Configure/reconfigure the swap chain.
    nxtSwapChainError (*Configure)(void* userData, nxtTextureFormat format, uint32_t width, uint32_t height);

    /// Acquire the next texture from the swap chain.
    nxtSwapChainError (*GetNextTexture)(void* userData, nxtSwapChainNextTexture* nextTexture);

    /// Present the last acquired texture to the screen.
    nxtSwapChainError (*Present)(void* userData);

    /// Each function is called with userData as its first argument.
    void* userData = nullptr;
} nxtSwapChainImplementation;

#ifdef NXT_ENABLE_BACKEND_D3D12
typedef struct {
} nxtWSIContextD3D12;
#endif

#ifdef NXT_ENABLE_BACKEND_METAL
typedef struct {
} nxtWSIContextMetal;
#endif

#ifdef NXT_ENABLE_BACKEND_OPENGL
typedef struct {
} nxtWSIContextGL;
#endif

#endif // NXT_WSI_H
