// Copyright 2024 The Dawn & Tint Authors
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

#ifndef WEBGPU_JNI_JNICONTEXT_H_
#define WEBGPU_JNI_JNICONTEXT_H_

#include <jni.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/utils/non_movable.h"

namespace dawn::kotlin_api {

// A helper context class to use while processing JNI calls.
//
//  - Frees memory that's allocated (or referenced) during the processing of the call.
class JNIContext : dawn::NonMovable {
  public:
    explicit JNIContext(JNIEnv* env);
    ~JNIContext();

    // Public members for convenience.
    JNIEnv* const env;
    JavaVM* jvm;

    const char* GetStringUTFChars(jstring s);
    const jint* GetIntArrayElements(jintArray a);

    template <typename T>
    T* Alloc() {
        static_assert(std::is_trivially_destructible_v<T>);
        static_assert(std::is_trivially_constructible_v<T>);
        return new (AllocateRaw(sizeof(T), alignof(T))) T();
    }

    template <typename T>
    T* AllocArray(size_t count) {
        static_assert(std::is_trivially_destructible_v<T>);
        static_assert(std::is_trivially_constructible_v<T>);

        if (count > std::numeric_limits<size_t>::max() / sizeof(T)) {
            env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"),
                          "Array size overflow");
            return nullptr;
        }

        T* ptr = static_cast<T*>(AllocateRaw(sizeof(T) * count, alignof(T)));
        if (ptr != nullptr) {
            std::memset(ptr, 0, sizeof(T) * count);
        }
        return ptr;
    }

    std::vector<std::shared_ptr<struct UserData>> recurringCallbacks;

  private:
    void* AllocateRaw(size_t size, size_t alignment);

    std::vector<std::pair<jstring, const char*>> mStringsToRelease;
    std::vector<std::pair<jintArray, jint*>> mIntArraysToRelease;

    static constexpr size_t kInlineBufferSize = 512;
    alignas(std::max_align_t) std::byte mInlineBuffer[kInlineBufferSize];
    size_t mInlineOffset = 0;

    struct BumpBlock {
        std::unique_ptr<std::byte[]> data;
        size_t size;
        size_t offset;
    };
    std::vector<BumpBlock> mBumpBlocks;
    static constexpr size_t kDefaultBumpBlockSize = 4096;
};

}  // namespace dawn::kotlin_api

#endif  // WEBGPU_JNI_JNICONTEXT_H_
