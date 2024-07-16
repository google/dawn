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
#include <utility>
#include <vector>

#include "dawn/common/NonMovable.h"

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

    const char* GetStringUTFChars(jstring s);
    const jint* GetIntArrayElements(jintArray a);

  private:
    std::vector<std::pair<jstring, const char*>> mStringsToRelease;
    std::vector<std::pair<jintArray, jint*>> mIntArraysToRelease;
};

}  // namespace dawn::kotlin_api

#endif  // WEBGPU_JNI_JNICONTEXT_H_
