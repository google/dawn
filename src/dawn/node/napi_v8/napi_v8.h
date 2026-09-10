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

#include <algorithm>
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

// Internal struct representing callback metadata passed to a native callback
struct napi_callback_info__ {
    const v8::FunctionCallbackInfo<v8::Value>* v8_info = nullptr;
    void* data = nullptr;
};

// Persistent binding metadata for a registered native function/method/accessor callback
struct CallbackBinding {
    napi_env env = nullptr;
    napi_callback callback = nullptr;
    void* user_data = nullptr;
};

// Internal struct representing a Node-API reference (napi_ref)
struct napi_ref__ {
    napi_env env = nullptr;
    v8::Global<v8::Value> handle;
    uint32_t ref_count = 0;
    void* native_object = nullptr;
    napi_finalize finalize_cb = nullptr;
    void* finalize_hint = nullptr;

    bool is_wrap_ref = false;
    bool is_userland_ref = false;

    napi_ref__(napi_env e,
               v8::Local<v8::Value> val,
               uint32_t count,
               void* native_obj = nullptr,
               napi_finalize fin_cb = nullptr,
               void* fin_hint = nullptr,
               bool wrap_ref = false,
               bool userland_ref = false);

    ~napi_ref__();

    void SetWeak();
    void ClearWeak();

    static void WeakCallback(const v8::WeakCallbackInfo<napi_ref__>& data);
};

// Internal struct representing a Node-API deferred promise (napi_deferred)
struct napi_deferred__ {
    v8::Global<v8::Promise::Resolver> resolver;
};

// Internal structs representing Node-API types stubbed for link compatibility
struct napi_async_context__ {};
struct napi_callback_scope__ {};
struct napi_async_work__ {};

// Instance data stored in napi_env
struct InstanceData {
    void* data = nullptr;
    napi_finalize finalize_cb = nullptr;
    void* finalize_hint = nullptr;
};

// Internal struct representing a Node-API environment (napi_env)
struct napi_env__ {
    v8::Isolate* isolate = nullptr;
    v8::Global<v8::Context> context;
    v8::Global<v8::Value> last_exception;
    napi_extended_error_info last_error{};
    std::vector<std::unique_ptr<napi_handle_scope__>> open_handle_scopes;
    std::vector<std::unique_ptr<CallbackBinding>> callback_bindings;
    std::vector<std::unique_ptr<napi_ref__>> references;
    std::vector<std::unique_ptr<napi_deferred__>> deferreds;
    InstanceData instance_data{};

    napi_env__(v8::Isolate* iso, v8::Local<v8::Context> ctx) : isolate(iso), context(iso, ctx) {
        ClearLastError();
    }

    ~napi_env__() {
        if (instance_data.finalize_cb != nullptr) {
            instance_data.finalize_cb(this, instance_data.data, instance_data.finalize_hint);
            instance_data.finalize_cb = nullptr;
        }

        // Finalize all remaining references that have an active finalizer.
        while (true) {
            auto it = std::find_if(references.rbegin(), references.rend(),
                                   [](const auto& r) { return r->finalize_cb != nullptr; });
            if (it == references.rend()) {
                break;
            }

            napi_ref__* ref = it->get();
            napi_finalize cb = ref->finalize_cb;
            void* native_object = ref->native_object;
            void* finalize_hint = ref->finalize_hint;

            // Reset the handle and clear finalize_cb BEFORE calling user code.
            ref->handle.Reset();
            ref->finalize_cb = nullptr;

            cb(this, native_object, finalize_hint);
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
