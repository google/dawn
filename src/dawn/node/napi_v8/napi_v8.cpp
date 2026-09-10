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

#include <algorithm>
#include <utility>

#include "src/utils/compiler.h"

napi_ref__::napi_ref__(napi_env e,
                       v8::Local<v8::Value> val,
                       uint32_t count,
                       void* native_obj,
                       napi_finalize fin_cb,
                       void* fin_hint,
                       bool wrap_ref,
                       bool userland_ref)
    : env(e),
      handle(e->isolate, val),
      ref_count(count),
      native_object(native_obj),
      finalize_cb(fin_cb),
      finalize_hint(fin_hint),
      is_wrap_ref(wrap_ref),
      is_userland_ref(userland_ref) {
    if (ref_count == 0) {
        SetWeak();
    }
}

napi_ref__::~napi_ref__() {
    handle.Reset();
}

void napi_ref__::SetWeak() {
    handle.SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);
}

void napi_ref__::ClearWeak() {
    handle.ClearWeak<void>();
}

void napi_ref__::WeakCallback(const v8::WeakCallbackInfo<napi_ref__>& data) {
    napi_ref__* self = data.GetParameter();
    napi_env env = self->env;
    void* native_object = self->native_object;
    void* finalize_hint = self->finalize_hint;
    napi_finalize finalize_cb = self->finalize_cb;

    // Reset the handle and clear finalize_cb before invoking the callback.
    // The finalize_cb may call napi_delete_reference, which deletes `self`.
    self->handle.Reset();
    self->finalize_cb = nullptr;

    if (finalize_cb != nullptr) {
        finalize_cb(env, native_object, finalize_hint);
    }
}

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

// Validates and extracts a v8::Object and its associated Context.
napi_status UnwrapObject(napi_env env,
                         napi_value object,
                         v8::Local<v8::Object>* out_obj,
                         v8::Local<v8::Context>* out_ctx) {
    *out_ctx = env->GetContext();
    v8::MaybeLocal<v8::Object> maybe_obj = dawn::napi_v8::ToV8(object)->ToObject(*out_ctx);
    if (maybe_obj.IsEmpty()) {
        return env->SetLastError(napi_object_expected, "An object was expected");
    }
    *out_obj = maybe_obj.ToLocalChecked();
    return napi_ok;
}

// Creates an internalized V8 string from a UTF-8 C string.
napi_status CreateInternalizedName(napi_env env,
                                   const char* utf8name,
                                   v8::Local<v8::String>* out_name) {
    if (!ValidateArgs(env, utf8name)) {
        return napi_invalid_arg;
    }
    v8::MaybeLocal<v8::String> maybe_name =
        v8::String::NewFromUtf8(env->isolate, utf8name, v8::NewStringType::kInternalized);
    if (maybe_name.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Failed to create property name");
    }
    *out_name = maybe_name.ToLocalChecked();
    return napi_ok;
}

// Generic property setter supporting Local<Value> names or uint32_t indices.
template <typename NameType>
napi_status WriteProperty(napi_env env,
                          napi_value object,
                          NameType name,
                          napi_value value,
                          const char* error_message) {
    if (!ValidateArgs(env, object, value)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Object> obj;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapObject(env, object, &obj, &ctx);
    if (status != napi_ok) {
        return status;
    }
    v8::Maybe<bool> res = obj->Set(ctx, name, dawn::napi_v8::ToV8(value));
    if (!res.FromMaybe(false)) {
        return env->SetLastError(napi_generic_failure, error_message);
    }
    return napi_ok;
}

// Generic property getter supporting Local<Value> names or uint32_t indices.
template <typename NameType>
napi_status ReadProperty(napi_env env,
                         napi_value object,
                         NameType name,
                         napi_value* result,
                         const char* error_message) {
    if (!ValidateArgs(env, object, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Object> obj;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapObject(env, object, &obj, &ctx);
    if (status != napi_ok) {
        return status;
    }
    v8::MaybeLocal<v8::Value> val = obj->Get(ctx, name);
    if (val.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, error_message);
    }
    *result = dawn::napi_v8::ToNapi(val.ToLocalChecked());
    return napi_ok;
}

// Generic property existence check supporting Local<Value> names or uint32_t indices.
template <typename NameType>
napi_status QueryProperty(napi_env env,
                          napi_value object,
                          NameType name,
                          bool* result,
                          const char* error_message) {
    if (!ValidateArgs(env, object, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Object> obj;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapObject(env, object, &obj, &ctx);
    if (status != napi_ok) {
        return status;
    }
    v8::Maybe<bool> has = obj->Has(ctx, name);
    if (has.IsNothing()) {
        return env->SetLastError(napi_generic_failure, error_message);
    }
    *result = has.FromJust();
    return napi_ok;
}

// V8 and Node-API use different callback conventions:
// - V8 callbacks use `void (const v8::FunctionCallbackInfo<v8::Value>&)` and pass user context via
//   an attached `v8::External` value in `v8_info.Data()`.
// - Node-API callbacks use `napi_value (*)(napi_env, napi_callback_info)` and retrieve user context
//   via `napi_get_cb_info()`.
//
// `NativeCallbackTrampoline` acts as the universal bridge: it unpacks the persistent
// `CallbackBinding` from the `v8::External`, constructs a lightweight `napi_callback_info__` stack
// wrapper referencing the active V8 callback arguments, executes the target `napi_callback`, and
// forwards the returned `napi_value` back to V8.
void NativeCallbackTrampoline(const v8::FunctionCallbackInfo<v8::Value>& v8_info) {
    v8::Local<v8::External> ext = v8_info.Data().As<v8::External>();
    auto* binding = static_cast<CallbackBinding*>(ext->Value(v8::kExternalPointerTypeTagDefault));
    napi_env env = binding->env;

    napi_callback_info__ invocation_info;
    invocation_info.v8_info = &v8_info;
    invocation_info.data = binding->user_data;

    v8::HandleScope scope(env->isolate);
    napi_value return_val = binding->callback(env, &invocation_info);
    if (return_val != nullptr) {
        v8_info.GetReturnValue().Set(dawn::napi_v8::ToV8(return_val));
    }
}

// Base helper to create a `v8::FunctionTemplate` from a v8::Name.
// To ensure the callback function pointer and user data remain valid across the environment's
// lifetime, an internal `CallbackBinding` structure is allocated and stored in
// `env->callback_bindings`. A pointer to this binding is attached to the `v8::FunctionTemplate` as
// a `v8::External`.
napi_status CreateFunctionTemplate(napi_env env,
                                   v8::Local<v8::Name> name,
                                   napi_callback callback,
                                   void* user_data,
                                   v8::Local<v8::FunctionTemplate>* out_template) {
    auto binding = std::make_unique<CallbackBinding>();
    binding->env = env;
    binding->callback = callback;
    binding->user_data = user_data;
    CallbackBinding* binding_ptr = binding.get();
    env->callback_bindings.push_back(std::move(binding));

    v8::Local<v8::External> ext =
        v8::External::New(env->isolate, binding_ptr, v8::kExternalPointerTypeTagDefault);
    auto function_template = v8::FunctionTemplate::New(env->isolate, NativeCallbackTrampoline, ext);
    if (!name.IsEmpty() && name->IsString()) {
        function_template->SetClassName(name.As<v8::String>());
    }
    *out_template = function_template;
    return napi_ok;
}

// UTF-8 string wrapper for CreateFunctionTemplate.
napi_status CreateFunctionTemplate(napi_env env,
                                   const char* utf8name,
                                   size_t length,
                                   napi_callback callback,
                                   void* user_data,
                                   v8::Local<v8::FunctionTemplate>* out_template) {
    v8::Local<v8::Name> name;
    if (utf8name != nullptr) {
        int len = (length == NAPI_AUTO_LENGTH) ? -1 : static_cast<int>(length);
        v8::MaybeLocal<v8::String> v8_name =
            v8::String::NewFromUtf8(env->isolate, utf8name, v8::NewStringType::kInternalized, len);
        if (v8_name.IsEmpty()) {
            return env->SetLastError(napi_generic_failure, "Failed to create function name");
        }
        name = v8_name.ToLocalChecked();
    }
    return CreateFunctionTemplate(env, name, callback, user_data, out_template);
}

// Extracts a property name from a Node-API property descriptor.
napi_status GetDescriptorName(napi_env env,
                              const napi_property_descriptor& desc,
                              v8::Local<v8::Name>* out_name) {
    if (desc.utf8name != nullptr) {
        v8::MaybeLocal<v8::String> name =
            v8::String::NewFromUtf8(env->isolate, desc.utf8name, v8::NewStringType::kInternalized);
        if (name.IsEmpty()) {
            return env->SetLastError(napi_generic_failure, "Failed to create property name");
        }
        *out_name = name.ToLocalChecked();
        return napi_ok;
    }
    if (desc.name != nullptr) {
        v8::Local<v8::Value> v8_name = dawn::napi_v8::ToV8(desc.name);
        if (!v8_name->IsName()) {
            return env->SetLastError(napi_name_expected, "A name or symbol was expected");
        }
        *out_name = v8_name.As<v8::Name>();
        return napi_ok;
    }
    return env->SetLastError(napi_invalid_arg, "Property descriptor missing name");
}

// Validates and extracts a v8::Function and its associated Context.
napi_status UnwrapFunction(napi_env env,
                           napi_value function_value,
                           v8::Local<v8::Function>* out_fn,
                           v8::Local<v8::Context>* out_ctx) {
    v8::Local<v8::Value> v8_func = dawn::napi_v8::ToV8(function_value);
    if (!v8_func->IsFunction()) {
        return env->SetLastError(napi_function_expected, "A function was expected");
    }
    *out_ctx = env->GetContext();
    *out_fn = v8_func.As<v8::Function>();
    return napi_ok;
}

// Unpacks and validates a Node-API argument array into a vector of V8 values.
napi_status UnpackArgs(napi_env env,
                       size_t argc,
                       const napi_value* argv,
                       std::vector<v8::Local<v8::Value>>* out_args) {
    if (argc > 0 && argv == nullptr) {
        return env->SetLastError(napi_invalid_arg, "Invalid argument: null argv");
    }
    out_args->resize(argc);
    for (size_t i = 0; i < argc; ++i) {
        // SAFETY: The caller guarantees argv points to an array with at least argc elements.
        napi_value arg = DAWN_UNSAFE_BUFFERS(argv[i]);
        if (arg == nullptr) {
            return env->SetLastError(napi_invalid_arg, "Invalid argument in argv");
        }
        (*out_args)[i] = dawn::napi_v8::ToV8(arg);
    }
    return napi_ok;
}

// Evaluates the result of a V8 function or constructor invocation within TryCatch.
template <typename T>
napi_status ProcessCallResult(napi_env env,
                              v8::TryCatch& try_catch,
                              v8::MaybeLocal<T> maybe_result,
                              napi_value* out_result) {
    if (try_catch.HasCaught()) {
        env->last_exception.Reset(env->isolate, try_catch.Exception());
        return env->SetLastError(napi_pending_exception,
                                 "An exception was thrown during execution");
    }
    if (maybe_result.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Function invocation failed");
    }
    if (out_result != nullptr) {
        *out_result = dawn::napi_v8::ToNapi(maybe_result.ToLocalChecked());
    }
    return napi_ok;
}

// Instantiates a JavaScript function from a native Node-API callback.
napi_status CreateCallbackFunction(napi_env env,
                                   v8::Local<v8::Context> ctx,
                                   v8::Local<v8::Name> name,
                                   napi_callback callback,
                                   void* data,
                                   v8::Local<v8::Function>* out_fn) {
    if (callback == nullptr) {
        return napi_ok;
    }
    v8::Local<v8::FunctionTemplate> function_template;
    napi_status status = CreateFunctionTemplate(env, name, callback, data, &function_template);
    if (status != napi_ok) {
        return status;
    }
    v8::MaybeLocal<v8::Function> maybe_fn = function_template->GetFunction(ctx);
    if (maybe_fn.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Failed to create callback function");
    }
    *out_fn = maybe_fn.ToLocalChecked();
    return napi_ok;
}
// Converts Node-API property attributes to V8 PropertyAttribute.
v8::PropertyAttribute ToV8PropertyAttribute(napi_property_attributes attributes) {
    int v8_attr = v8::None;
    if ((attributes & napi_writable) == 0) {
        v8_attr |= v8::ReadOnly;
    }
    if ((attributes & napi_enumerable) == 0) {
        v8_attr |= v8::DontEnum;
    }
    if ((attributes & napi_configurable) == 0) {
        v8_attr |= v8::DontDelete;
    }
    return static_cast<v8::PropertyAttribute>(v8_attr);
}
// Attaches a single property descriptor to an existing JavaScript object.
napi_status AttachObjectProperty(napi_env env,
                                 v8::Local<v8::Context> ctx,
                                 v8::Local<v8::Object> obj,
                                 const napi_property_descriptor& desc) {
    v8::Local<v8::Name> name;
    napi_status status = GetDescriptorName(env, desc, &name);
    if (status != napi_ok) {
        return status;
    }

    v8::PropertyAttribute v8_attr = ToV8PropertyAttribute(desc.attributes);
    if (desc.method != nullptr) {
        v8::Local<v8::Function> method_fn;
        status = CreateCallbackFunction(env, ctx, name, desc.method, desc.data, &method_fn);
        if (status != napi_ok) {
            return status;
        }
        if (obj->DefineOwnProperty(ctx, name, method_fn, v8_attr).IsNothing()) {
            return env->SetLastError(napi_generic_failure, "Failed to define method");
        }
    } else if (desc.getter != nullptr || desc.setter != nullptr) {
        v8::Local<v8::Function> getter_fn;
        status = CreateCallbackFunction(env, ctx, {}, desc.getter, desc.data, &getter_fn);
        if (status != napi_ok) {
            return status;
        }
        v8::Local<v8::Function> setter_fn;
        status = CreateCallbackFunction(env, ctx, {}, desc.setter, desc.data, &setter_fn);
        if (status != napi_ok) {
            return status;
        }
        obj->SetAccessorProperty(name, getter_fn, setter_fn, v8_attr);
    } else if (desc.value != nullptr) {
        if (obj->DefineOwnProperty(ctx, name, dawn::napi_v8::ToV8(desc.value), v8_attr)
                .IsNothing()) {
            return env->SetLastError(napi_generic_failure, "Failed to set property value");
        }
    }
    return napi_ok;
}

enum class ErrorType {
    Error,
    TypeError,
    RangeError,
};

// Helper to create JavaScript Error objects.
napi_status CreateError(napi_env env,
                        ErrorType error_type,
                        napi_value code,
                        napi_value msg,
                        napi_value* result) {
    if (!ValidateArgs(env, msg, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Value> v8_msg = dawn::napi_v8::ToV8(msg);
    if (!v8_msg->IsString()) {
        return env->SetLastError(napi_string_expected, "A string was expected for error message");
    }
    v8::Local<v8::String> str_msg = v8_msg.As<v8::String>();
    v8::Local<v8::Value> err;
    switch (error_type) {
        case ErrorType::Error:
            err = v8::Exception::Error(str_msg);
            break;
        case ErrorType::TypeError:
            err = v8::Exception::TypeError(str_msg);
            break;
        case ErrorType::RangeError:
            err = v8::Exception::RangeError(str_msg);
            break;
    }
    if (code != nullptr) {
        v8::Local<v8::Context> ctx = env->GetContext();
        v8::Local<v8::String> code_name;
        napi_status status = CreateInternalizedName(env, "code", &code_name);
        if (status != napi_ok) {
            return status;
        }
        if (err.As<v8::Object>()->Set(ctx, code_name, dawn::napi_v8::ToV8(code)).IsNothing()) {
            return env->SetLastError(napi_generic_failure, "Failed to set error code");
        }
    }
    *result = dawn::napi_v8::ToNapi(err);
    return napi_ok;
}

napi_status GetWrapObject(napi_env env, napi_value js_object, v8::Local<v8::Object>* out_obj) {
    if (!ValidateArgs(env, js_object)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Value> v8_val = dawn::napi_v8::ToV8(js_object);
    if (!v8_val->IsObject()) {
        return env->SetLastError(napi_object_expected, "An object was expected");
    }
    v8::Local<v8::Object> obj = v8_val.As<v8::Object>();
    if (obj->InternalFieldCount() == 0) {
        return env->SetLastError(napi_invalid_arg, "Object has no internal fields");
    }
    *out_obj = obj;
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

// ============================================================================
// Objects & Properties
// ============================================================================

napi_status napi_create_object(napi_env env, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Object::New(env->isolate));
    return napi_ok;
}

napi_status napi_get_property_names(napi_env env, napi_value object, napi_value* result) {
    if (!ValidateArgs(env, object, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Object> obj;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapObject(env, object, &obj, &ctx);
    if (status != napi_ok) {
        return status;
    }
    v8::MaybeLocal<v8::Array> names = obj->GetPropertyNames(ctx);
    if (names.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Failed to get property names");
    }
    *result = ToNapi(names.ToLocalChecked());
    return napi_ok;
}

napi_status napi_set_property(napi_env env, napi_value object, napi_value name, napi_value value) {
    if (!ValidateArgs(env, name)) {
        return napi_invalid_arg;
    }
    return WriteProperty(env, object, ToV8(name), value, "Failed to set property");
}

napi_status napi_get_property(napi_env env,
                              napi_value object,
                              napi_value name,
                              napi_value* result) {
    if (!ValidateArgs(env, name)) {
        return napi_invalid_arg;
    }
    return ReadProperty(env, object, ToV8(name), result, "Failed to get property");
}

napi_status napi_has_property(napi_env env, napi_value object, napi_value name, bool* result) {
    if (!ValidateArgs(env, name)) {
        return napi_invalid_arg;
    }
    return QueryProperty(env, object, ToV8(name), result, "Failed to check property");
}

napi_status napi_set_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    napi_value value) {
    v8::Local<v8::String> name;
    napi_status status = CreateInternalizedName(env, utf8name, &name);
    if (status != napi_ok) {
        return status;
    }
    return WriteProperty(env, object, name, value, "Failed to set named property");
}

napi_status napi_get_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    napi_value* result) {
    v8::Local<v8::String> name;
    napi_status status = CreateInternalizedName(env, utf8name, &name);
    if (status != napi_ok) {
        return status;
    }
    return ReadProperty(env, object, name, result, "Failed to get named property");
}

napi_status napi_has_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    bool* result) {
    v8::Local<v8::String> name;
    napi_status status = CreateInternalizedName(env, utf8name, &name);
    if (status != napi_ok) {
        return status;
    }
    return QueryProperty(env, object, name, result, "Failed to check named property");
}

// ============================================================================
// Arrays & Indexed Properties
// ============================================================================

napi_status napi_create_array(napi_env env, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Array::New(env->isolate));
    return napi_ok;
}

napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = ToNapi(v8::Array::New(env->isolate, static_cast<int>(length)));
    return napi_ok;
}

napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Value> v8_val = ToV8(value);
    if (!v8_val->IsArray()) {
        return env->SetLastError(napi_array_expected, "An array was expected");
    }
    *result = v8_val.As<v8::Array>()->Length();
    return napi_ok;
}

napi_status napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value* result) {
    return ReadProperty(env, object, index, result, "Failed to get element");
}

napi_status napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value) {
    return WriteProperty(env, object, index, value, "Failed to set element");
}

// ============================================================================
// Functions, Classes & Callbacks
// ============================================================================

napi_status napi_create_function(napi_env env,
                                 const char* utf8name,
                                 size_t length,
                                 napi_callback cb,
                                 void* data,
                                 napi_value* result) {
    if (!ValidateArgs(env, cb, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::FunctionTemplate> function_template;
    napi_status status =
        CreateFunctionTemplate(env, utf8name, length, cb, data, &function_template);
    if (status != napi_ok) {
        return status;
    }
    v8::MaybeLocal<v8::Function> fn = function_template->GetFunction(env->GetContext());
    if (fn.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Failed to create function");
    }
    *result = dawn::napi_v8::ToNapi(fn.ToLocalChecked());
    return napi_ok;
}

napi_status napi_call_function(napi_env env,
                               napi_value recv,
                               napi_value func,
                               size_t argc,
                               const napi_value* argv,
                               napi_value* result) {
    if (!ValidateArgs(env, func)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Function> fn;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapFunction(env, func, &fn, &ctx);
    if (status != napi_ok) {
        return status;
    }

    std::vector<v8::Local<v8::Value>> args;
    status = UnpackArgs(env, argc, argv, &args);
    if (status != napi_ok) {
        return status;
    }

    v8::Local<v8::Value> v8_recv = (recv != nullptr)
                                       ? dawn::napi_v8::ToV8(recv)
                                       : v8::Local<v8::Value>(v8::Undefined(env->isolate));

    v8::TryCatch try_catch(env->isolate);
    v8::MaybeLocal<v8::Value> ret = fn->Call(ctx, v8_recv, static_cast<int>(argc), args.data());
    return ProcessCallResult(env, try_catch, ret, result);
}

napi_status napi_get_cb_info(napi_env env,
                             napi_callback_info cbinfo,
                             size_t* argc,
                             napi_value* argv,
                             napi_value* this_arg,
                             void** data) {
    if (!ValidateArgs(env, cbinfo)) {
        return napi_invalid_arg;
    }
    if (data != nullptr) {
        *data = cbinfo->data;
    }
    const auto& info = *(cbinfo->v8_info);
    if (this_arg != nullptr) {
        *this_arg = dawn::napi_v8::ToNapi(info.This());
    }
    if (argc != nullptr) {
        size_t available_argc = static_cast<size_t>(info.Length());
        if (argv != nullptr) {
            size_t copy_count = std::min(*argc, available_argc);
            for (size_t i = 0; i < copy_count; ++i) {
                // SAFETY: When argv is non-null, the caller guarantees it points to a buffer of at
                // least *argc elements.
                DAWN_UNSAFE_BUFFERS(argv[i]) = dawn::napi_v8::ToNapi(info[static_cast<int>(i)]);
            }
            for (size_t i = copy_count; i < *argc; ++i) {
                // SAFETY: When argv is non-null, the caller guarantees it points to a buffer of at
                // least *argc elements.
                DAWN_UNSAFE_BUFFERS(argv[i]) = dawn::napi_v8::ToNapi(v8::Undefined(env->isolate));
            }
        }
        *argc = available_argc;
    }
    return napi_ok;
}

napi_status napi_new_instance(napi_env env,
                              napi_value constructor,
                              size_t argc,
                              const napi_value* argv,
                              napi_value* result) {
    if (!ValidateArgs(env, constructor, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Function> ctor;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapFunction(env, constructor, &ctor, &ctx);
    if (status != napi_ok) {
        return status;
    }

    std::vector<v8::Local<v8::Value>> args;
    status = UnpackArgs(env, argc, argv, &args);
    if (status != napi_ok) {
        return status;
    }

    v8::TryCatch try_catch(env->isolate);
    v8::MaybeLocal<v8::Object> ret = ctor->NewInstance(ctx, static_cast<int>(argc), args.data());
    return ProcessCallResult(env, try_catch, ret, result);
}

napi_status napi_define_class(napi_env env,
                              const char* utf8name,
                              size_t length,
                              napi_callback constructor,
                              void* data,
                              size_t property_count,
                              const napi_property_descriptor* properties,
                              napi_value* result) {
    if (!ValidateArgs(env, constructor, result)) {
        return napi_invalid_arg;
    }
    if (property_count > 0 && properties == nullptr) {
        return env->SetLastError(napi_invalid_arg, "Invalid argument: null properties");
    }

    v8::Local<v8::FunctionTemplate> class_template;
    napi_status status =
        CreateFunctionTemplate(env, utf8name, length, constructor, data, &class_template);
    if (status != napi_ok) {
        return status;
    }
    class_template->InstanceTemplate()->SetInternalFieldCount(1);

    v8::Local<v8::Context> ctx = env->GetContext();
    v8::MaybeLocal<v8::Function> maybe_ctor = class_template->GetFunction(ctx);
    if (maybe_ctor.IsEmpty()) {
        return env->SetLastError(napi_generic_failure, "Failed to create class constructor");
    }
    v8::Local<v8::Function> ctor_fn = maybe_ctor.ToLocalChecked();

    v8::Local<v8::Object> proto_obj;
    if (property_count > 0) {
        v8::Local<v8::String> proto_name;
        status = CreateInternalizedName(env, "prototype", &proto_name);
        if (status != napi_ok) {
            return status;
        }
        v8::Local<v8::Value> proto_val;
        if (!ctor_fn->Get(ctx, proto_name).ToLocal(&proto_val) || !proto_val->IsObject()) {
            return env->SetLastError(napi_generic_failure, "Failed to get class prototype object");
        }
        proto_obj = proto_val.As<v8::Object>();
    }

    for (size_t i = 0; i < property_count; ++i) {
        // SAFETY: The caller guarantees properties points to an array of at least property_count
        // descriptors.
        const napi_property_descriptor& prop = DAWN_UNSAFE_BUFFERS(properties[i]);
        bool is_static = (prop.attributes & napi_static) != 0;
        v8::Local<v8::Object> target = is_static ? ctor_fn : proto_obj;
        status = AttachObjectProperty(env, ctx, target, prop);
        if (status != napi_ok) {
            return status;
        }
    }

    *result = dawn::napi_v8::ToNapi(ctor_fn);
    return napi_ok;
}

napi_status napi_define_properties(napi_env env,
                                   napi_value object,
                                   size_t property_count,
                                   const napi_property_descriptor* properties) {
    if (!ValidateArgs(env, object)) {
        return napi_invalid_arg;
    }
    if (property_count > 0 && properties == nullptr) {
        return env->SetLastError(napi_invalid_arg, "Invalid argument: null properties");
    }

    v8::Local<v8::Object> obj;
    v8::Local<v8::Context> ctx;
    napi_status status = UnwrapObject(env, object, &obj, &ctx);
    if (status != napi_ok) {
        return status;
    }

    for (size_t i = 0; i < property_count; ++i) {
        // SAFETY: The caller guarantees properties points to an array of at least property_count
        // descriptors.
        const napi_property_descriptor& prop = DAWN_UNSAFE_BUFFERS(properties[i]);
        status = AttachObjectProperty(env, ctx, obj, prop);
        if (status != napi_ok) {
            return status;
        }
    }
    return napi_ok;
}

// ============================================================================
// Exceptions & Errors
// ============================================================================

napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result) {
    return CreateError(env, ErrorType::Error, code, msg, result);
}

napi_status napi_create_type_error(napi_env env,
                                   napi_value code,
                                   napi_value msg,
                                   napi_value* result) {
    return CreateError(env, ErrorType::TypeError, code, msg, result);
}

napi_status napi_create_range_error(napi_env env,
                                    napi_value code,
                                    napi_value msg,
                                    napi_value* result) {
    return CreateError(env, ErrorType::RangeError, code, msg, result);
}

napi_status napi_throw(napi_env env, napi_value error) {
    if (!ValidateArgs(env, error)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Value> v8_err = dawn::napi_v8::ToV8(error);
    env->isolate->ThrowException(v8_err);
    env->last_exception.Reset(env->isolate, v8_err);
    return napi_ok;
}

napi_status napi_is_exception_pending(napi_env env, bool* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    *result = !env->last_exception.IsEmpty();
    return napi_ok;
}

napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    if (!env->last_exception.IsEmpty()) {
        *result = dawn::napi_v8::ToNapi(env->last_exception.Get(env->isolate));
        env->last_exception.Reset();
    } else {
        *result = dawn::napi_v8::ToNapi(v8::Undefined(env->isolate));
    }
    return napi_ok;
}

napi_status napi_is_error(napi_env env, napi_value value, bool* result) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    *result = dawn::napi_v8::ToV8(value)->IsNativeError();
    return napi_ok;
}

// ============================================================================
// References, ObjectWrap & Instance Data
// ============================================================================

napi_status napi_create_reference(napi_env env,
                                  napi_value value,
                                  uint32_t initial_refcount,
                                  napi_ref* result) {
    if (!ValidateArgs(env, value, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Value> v8_val = dawn::napi_v8::ToV8(value);
    auto ref = std::make_unique<napi_ref__>(env, v8_val, initial_refcount);
    *result = ref.get();
    env->references.push_back(std::move(ref));
    return napi_ok;
}

napi_status napi_delete_reference(napi_env env, napi_ref ref) {
    if (!ValidateArgs(env, ref)) {
        return napi_invalid_arg;
    }
    size_t erased = std::erase_if(
        env->references, [ref](const std::unique_ptr<napi_ref__>& r) { return r.get() == ref; });
    if (erased > 0) {
        return napi_ok;
    }
    return env->SetLastError(napi_invalid_arg, "Reference not found");
}

napi_status napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result) {
    if (!ValidateArgs(env, ref)) {
        return napi_invalid_arg;
    }
    if (ref->ref_count == 0) {
        ref->ClearWeak();
    }
    ref->ref_count++;
    if (result != nullptr) {
        *result = ref->ref_count;
    }
    return napi_ok;
}

napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result) {
    if (!ValidateArgs(env, ref)) {
        return napi_invalid_arg;
    }
    if (ref->ref_count == 0) {
        return env->SetLastError(napi_generic_failure, "Cannot unref a reference with refcount 0");
    }
    ref->ref_count--;
    if (ref->ref_count == 0) {
        ref->SetWeak();
    }
    if (result != nullptr) {
        *result = ref->ref_count;
    }
    return napi_ok;
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result) {
    if (!ValidateArgs(env, ref, result)) {
        return napi_invalid_arg;
    }
    if (ref->handle.IsEmpty()) {
        *result = nullptr;
    } else {
        *result = dawn::napi_v8::ToNapi(ref->handle.Get(env->isolate));
    }
    return napi_ok;
}

napi_status napi_wrap(napi_env env,
                      napi_value js_object,
                      void* native_object,
                      napi_finalize finalize_cb,
                      void* finalize_hint,
                      napi_ref* result) {
    v8::Local<v8::Object> obj;
    napi_status status = GetWrapObject(env, js_object, &obj);
    if (status != napi_ok) {
        return status;
    }
    if (obj->GetAlignedPointerFromInternalField(0, v8::kEmbedderDataTypeTagDefault) != nullptr) {
        return env->SetLastError(napi_invalid_arg, "Object is already wrapped");
    }
    obj->SetAlignedPointerInInternalField(0, native_object, v8::kEmbedderDataTypeTagDefault);
    if (finalize_cb != nullptr || result != nullptr) {
        auto ref = std::make_unique<napi_ref__>(env, obj, 0, native_object, finalize_cb,
                                                finalize_hint, /*wrap_ref=*/true,
                                                /*userland_ref=*/result != nullptr);
        if (result != nullptr) {
            *result = ref.get();
        }
        env->references.push_back(std::move(ref));
    }
    return napi_ok;
}

napi_status napi_unwrap(napi_env env, napi_value js_object, void** result) {
    if (!ValidateArgs(env, result)) {
        return napi_invalid_arg;
    }
    v8::Local<v8::Object> obj;
    napi_status status = GetWrapObject(env, js_object, &obj);
    if (status != napi_ok) {
        return status;
    }
    *result = obj->GetAlignedPointerFromInternalField(0, v8::kEmbedderDataTypeTagDefault);
    return napi_ok;
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object, void** result) {
    v8::Local<v8::Object> obj;
    napi_status status = GetWrapObject(env, js_object, &obj);
    if (status != napi_ok) {
        return status;
    }
    if (result != nullptr) {
        *result = obj->GetAlignedPointerFromInternalField(0, v8::kEmbedderDataTypeTagDefault);
    }
    obj->SetAlignedPointerInInternalField(0, nullptr, v8::kEmbedderDataTypeTagDefault);

    // Find and detach the associated wrap reference
    auto it = std::find_if(env->references.begin(), env->references.end(), [&](const auto& r) {
        return r->is_wrap_ref && !r->handle.IsEmpty() && r->handle.Get(env->isolate) == obj;
    });
    if (it != env->references.end()) {
        napi_ref__* ref = it->get();
        // Clear the finalizer so that it will never run on the detached native object.
        ref->finalize_cb = nullptr;
        ref->native_object = nullptr;
        ref->is_wrap_ref = false;

        if (!ref->is_userland_ref) {
            // Internal runtime reference: erasing it destroys the unique_ptr, whose
            // destructor calls handle.Reset(), deregistering the weak callback in V8.
            env->references.erase(it);
        }
    }
    return napi_ok;
}

napi_status napi_set_instance_data(napi_env env,
                                   void* data,
                                   napi_finalize finalize_cb,
                                   void* finalize_hint) {
    if (!ValidateArgs(env)) {
        return napi_invalid_arg;
    }
    if (env->instance_data.finalize_cb != nullptr) {
        env->instance_data.finalize_cb(env, env->instance_data.data,
                                       env->instance_data.finalize_hint);
    }
    env->instance_data.data = data;
    env->instance_data.finalize_cb = finalize_cb;
    env->instance_data.finalize_hint = finalize_hint;
    return napi_ok;
}

napi_status napi_get_instance_data(napi_env env, void** data) {
    if (!ValidateArgs(env, data)) {
        return napi_invalid_arg;
    }
    *data = env->instance_data.data;
    return napi_ok;
}

}  // extern "C"
