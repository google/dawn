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

#ifndef SRC_DAWN_NODE_NAPI_V8_NAPI_V8_H_
#define SRC_DAWN_NODE_NAPI_V8_NAPI_V8_H_

#include <js_native_api.h>
#include <js_native_api_types.h>
#include <node_api.h>
#include <node_api_types.h>

#include <bit>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wunique-object-duplication"
#pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
#include <v8.h>
#pragma clang diagnostic pop

// Internal struct representing a handle scope
struct napi_handle_scope__ {
    v8::HandleScope scope;
    explicit napi_handle_scope__(v8::Isolate* isolate) : scope(isolate) {}
};

// Internal struct representing a Node-API environment (napi_env)
struct napi_env__ {
    v8::Isolate* isolate = nullptr;
    v8::Global<v8::Context> context;
    napi_extended_error_info last_error{};
    std::vector<std::unique_ptr<napi_handle_scope__>> open_handle_scopes;

    napi_env__(v8::Isolate* iso, v8::Local<v8::Context> ctx) : isolate(iso), context(iso, ctx) {
        ClearLastError();
    }

    ~napi_env__() {
        // Automatically close all remaining open handle scopes in LIFO order
        while (!open_handle_scopes.empty()) {
            open_handle_scopes.pop_back();
        }
    }

    v8::Local<v8::Context> GetContext() const { return context.Get(isolate); }

    void ClearLastError() {
        last_error.error_code = napi_ok;
        last_error.engine_error_code = 0;
        last_error.engine_reserved = nullptr;
        last_error.error_message = nullptr;
    }

    napi_status SetLastError(napi_status status,
                             const char* error_message = nullptr,
                             uint32_t engine_error_code = 0) {
        last_error.error_code = status;
        last_error.engine_error_code = engine_error_code;
        last_error.engine_reserved = nullptr;
        last_error.error_message = error_message;
        return status;
    }
};

// Inline handle conversion functions between V8 and Node-API.
// v8::Local<v8::Value> is guaranteed by V8 to be a trivially copyable, pointer-sized
// V8_TRIVIAL_ABI handle pointing to an active HandleScope slot.
namespace dawn::napi_v8 {

static_assert(sizeof(napi_value) == sizeof(v8::Local<v8::Value>));
static_assert(std::is_trivially_copyable_v<v8::Local<v8::Value>>);

inline napi_value ToNapi(v8::Local<v8::Value> val) {
    return std::bit_cast<napi_value>(val);
}

inline v8::Local<v8::Value> ToV8(napi_value val) {
    return std::bit_cast<v8::Local<v8::Value>>(val);
}

// Environment factory helpers
inline napi_env CreateEnv(v8::Isolate* isolate, v8::Local<v8::Context> context) {
    return new napi_env__(isolate, context);
}

inline void DestroyEnv(napi_env env) {
    delete env;
}

}  // namespace dawn::napi_v8

#endif  // SRC_DAWN_NODE_NAPI_V8_NAPI_V8_H_
