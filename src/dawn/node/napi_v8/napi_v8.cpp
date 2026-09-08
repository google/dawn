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

#include "src/dawn/node/napi_v8/napi_v8.h"

#include "src/utils/compiler.h"

namespace {

// Validates that the environment pointer and all required argument pointers are non-null.
// Clears any previous error on the environment if the environment is valid.
template <typename... Args>
bool ValidateArgs(napi_env env, Args... args) {
    if (env == nullptr) {
        return false;
    }
    env->ClearLastError();
    if (((args == nullptr) || ...)) {
        env->SetLastError(napi_invalid_arg, "Invalid argument: null pointer");
        return false;
    }
    return true;
}

// Converts a JavaScript value into a numeric primitive type using a conversion callback.
template <typename V8Type, typename OutType, typename ConvertFn>
napi_status ExtractNumber(napi_env env,
                          napi_value value,
                          OutType* result,
                          const char* error_message,
                          ConvertFn&& convert) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    v8::MaybeLocal<V8Type> maybe_num = convert(dawn::napi_v8::ToV8(value), env->GetContext());
    if (maybe_num.IsEmpty()) {
        return env->SetLastError(napi_number_expected, error_message);
    }
    *result = static_cast<OutType>(maybe_num.ToLocalChecked()->Value());
    return napi_ok;
}

// Coerces a JavaScript value into another JavaScript value type.
template <typename V8Type, typename ConvertFn>
napi_status CoerceTo(napi_env env,
                     napi_value value,
                     napi_value* result,
                     const char* error_message,
                     ConvertFn&& convert) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    v8::MaybeLocal<V8Type> maybe_val = convert(dawn::napi_v8::ToV8(value), env->GetContext());
    if (maybe_val.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, error_message);
    }
    *result = dawn::napi_v8::ToNapi(maybe_val.ToLocalChecked());
    return napi_ok;
}

}  // namespace

using dawn::napi_v8::ToNapi;
using dawn::napi_v8::ToV8;

extern "C" {

// ============================================================================
// Error Handling & Environment Info
// ============================================================================

napi_status napi_get_last_error_info(node_api_nogc_env env,
                                     const napi_extended_error_info** result) {
    if (env == nullptr || result == nullptr) {
        return napi_invalid_arg;
    }
    *result = &(env->last_error);
    return napi_ok;
}

// ============================================================================
// Handle Scopes
// ============================================================================

napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    env->open_handle_scopes.push_back(std::make_unique<napi_handle_scope__>(env->isolate));
    *result = env->open_handle_scopes.back().get();
    return napi_ok;
}

napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
    if (!ValidateArgs(env, scope)) {
        return napi_invalid_arg;
    }
    if (env->open_handle_scopes.empty() || env->open_handle_scopes.back().get() != scope) {
        return env->SetLastError(napi_handle_scope_mismatch, "Handle scope closed out of order");
    }
    env->open_handle_scopes.pop_back();
    return napi_ok;
}

// ============================================================================
// Primitives & Singletons
// ============================================================================

napi_status napi_get_undefined(napi_env env, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Undefined(env->isolate));
    return napi_ok;
}

napi_status napi_get_null(napi_env env, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Null(env->isolate));
    return napi_ok;
}

napi_status napi_get_global(napi_env env, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(env->GetContext()->Global());
    return napi_ok;
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Boolean::New(env->isolate, value));
    return napi_ok;
}

napi_status napi_create_double(napi_env env, double value, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Number::New(env->isolate, value));
    return napi_ok;
}

napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Integer::New(env->isolate, value));
    return napi_ok;
}

napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Integer::NewFromUnsigned(env->isolate, value));
    return napi_ok;
}

napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Number::New(env->isolate, static_cast<double>(value)));
    return napi_ok;
}

napi_status napi_create_string_utf8(napi_env env,
                                    const char* str,
                                    size_t length,
                                    napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    int len = (length == NAPI_AUTO_LENGTH) ? -1 : static_cast<int>(length);
    v8::MaybeLocal<v8::String> v8_str =
        v8::String::NewFromUtf8(env->isolate, str ? str : "", v8::NewStringType::kNormal, len);
    if (v8_str.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Failed to create UTF-8 string");
    }
    *result = ToNapi(v8_str.ToLocalChecked());
    return napi_ok;
}

napi_status napi_create_symbol(napi_env env, napi_value description, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::String> desc;
    if (description != nullptr) {
        desc = ToV8(description).As<v8::String>();
    }
    *result = ToNapi(v8::Symbol::New(env->isolate, desc));
    return napi_ok;
}

napi_status napi_get_value_double(napi_env env, napi_value value, double* result) {
    return ExtractNumber<v8::Number>(env, value, result, "A number was expected",
                                     [](auto v, auto ctx) { return v->ToNumber(ctx); });
}

napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result) {
    return ExtractNumber<v8::Int32>(env, value, result, "A 32-bit integer was expected",
                                    [](auto v, auto ctx) { return v->ToInt32(ctx); });
}

napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result) {
    return ExtractNumber<v8::Uint32>(env, value, result, "An unsigned 32-bit integer was expected",
                                     [](auto v, auto ctx) { return v->ToUint32(ctx); });
}

napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result) {
    return ExtractNumber<v8::Integer>(env, value, result, "An integer was expected",
                                      [](auto v, auto ctx) { return v->ToInteger(ctx); });
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    *result = ToV8(value)->BooleanValue(env->isolate);
    return napi_ok;
}

napi_status napi_get_value_string_utf8(napi_env env,
                                       napi_value value,
                                       char* buf,
                                       size_t bufsize,
                                       size_t* result) {
    if (!ValidateArgs(env, value)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Value> val = ToV8(value);
    if (!val->IsString()) {
        return env->SetLastError(napi_string_expected, "A string was expected");
    }

    v8::Local<v8::String> str = val.As<v8::String>();
    size_t utf8_len = static_cast<size_t>(str->Utf8Length(env->isolate));
    if (buf == nullptr) {
        if (result == nullptr) {
            return env->SetLastError(napi_invalid_arg, "Invalid argument: null pointer");
        }
        *result = utf8_len;
    } else if (bufsize == 0) {
        if (result != nullptr) {
            *result = 0;
        }
    } else {
        size_t written = str->WriteUtf8(env->isolate, buf, bufsize - 1,
                                        v8::String::WriteFlags::kReplaceInvalidUtf8);
        // SAFETY: written is guaranteed to be <= bufsize - 1 by WriteUtf8, so written is a valid
        // index in buf.
        DAWN_UNSAFE_BUFFERS(buf[written] = '\0');
        if (result != nullptr) {
            *result = written;
        }
    }
    return napi_ok;
}

// ============================================================================
// Type Coercions, Type Checking & Equality
// ============================================================================

napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype* result) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }

    v8::Local<v8::Value> val = ToV8(value);
    if (val->IsNumber()) {
        *result = napi_number;
    } else if (val->IsString()) {
        *result = napi_string;
    } else if (val->IsFunction()) {
        *result = napi_function;
    } else if (val->IsArray()) {
        *result = napi_object;
    } else if (val->IsBoolean()) {
        *result = napi_boolean;
    } else if (val->IsUndefined()) {
        *result = napi_undefined;
    } else if (val->IsNull()) {
        *result = napi_null;
    } else if (val->IsSymbol()) {
        *result = napi_symbol;
    } else if (val->IsBigInt()) {
        *result = napi_bigint;
    } else if (val->IsExternal()) {
        *result = napi_external;
    } else if (val->IsObject()) {
        *result = napi_object;
    } else {
        *result = napi_undefined;
    }
    return napi_ok;
}

napi_status napi_coerce_to_bool(napi_env env, napi_value value, napi_value* result) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(ToV8(value)->ToBoolean(env->isolate));
    return napi_ok;
}

napi_status napi_coerce_to_number(napi_env env, napi_value value, napi_value* result) {
    return CoerceTo<v8::Number>(env, value, result, "Failed to coerce value to number",
                                [](auto v, auto ctx) { return v->ToNumber(ctx); });
}

napi_status napi_coerce_to_string(napi_env env, napi_value value, napi_value* result) {
    return CoerceTo<v8::String>(env, value, result, "Failed to coerce value to string",
                                [](auto v, auto ctx) { return v->ToString(ctx); });
}

napi_status napi_coerce_to_object(napi_env env, napi_value value, napi_value* result) {
    return CoerceTo<v8::Object>(env, value, result, "Failed to coerce value to object",
                                [](auto v, auto ctx) { return v->ToObject(ctx); });
}

napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result) {
    if (!ValidateArgs(env, lhs, rhs, result)) {
        return napi_invalid_arg;
    }
    *result = ToV8(lhs)->StrictEquals(ToV8(rhs));
    return napi_ok;
}

}  // extern "C"
