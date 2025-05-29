// Copyright 2025 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/dawn/node/binding/EventTarget.h"

#include <vector>

namespace wgpu::binding {

// Note: We don't cache many of these lookups since the developer
// is free to patch methods between calls.

// Creates a native EventTarget
EventTarget::EventTarget(Napi::Env env) {
    Napi::Object global = env.Global();
    Napi::Value eventTargetValue = global.Get("EventTarget");
    Napi::Function eventTargetConstructor = eventTargetValue.As<Napi::Function>();
    Napi::Object jsEventTarget = eventTargetConstructor.New({});
    eventTargetRef_ = Napi::Persistent(jsEventTarget);
}
EventTarget::~EventTarget() = default;

// Forward to our internal EventTarget.
void EventTarget::addEventListener(
    Napi::Env env,
    std::string type,
    std::optional<interop::EventListener> callback,
    std::optional<std::variant<interop::AddEventListenerOptions, bool>> options) {
    Napi::Object jsEventTarget = eventTargetRef_.Value();
    Napi::Value addListenerValue = jsEventTarget.Get("addEventListener");
    Napi::Function addListenerFunc = addListenerValue.As<Napi::Function>();

    std::vector<Napi::Value> args({
        interop::ToJS(env, type),
        interop::ToJS(env, callback),
        interop::ToJS(env, options),
    });
    addListenerFunc.Call(jsEventTarget, args);
}

// Forward to our internal EventTarget.
void EventTarget::removeEventListener(
    Napi::Env env,
    std::string type,
    std::optional<interop::EventListener> callback,
    std::optional<std::variant<interop::EventListenerOptions, bool>> options) {
    Napi::Object jsEventTarget = eventTargetRef_.Value();
    Napi::Value removeListenerValue = jsEventTarget.Get("removeEventListener");
    Napi::Function removeListenerFunc = removeListenerValue.As<Napi::Function>();

    std::vector<Napi::Value> args{
        interop::ToJS(env, type),
        interop::ToJS(env, callback),
        interop::ToJS(env, options),
    };
    removeListenerFunc.Call(jsEventTarget, args);
}

// Forward to our internal EventTarget.
bool EventTarget::dispatchEvent(Napi::Env env, interop::Event event) {
    Napi::Object jsEventTarget = eventTargetRef_.Value();
    Napi::Value dispatchEventValue = jsEventTarget.Get("dispatchEvent");
    Napi::Function dispatchEventFunc = dispatchEventValue.As<Napi::Function>();

    std::vector<Napi::Value> args{interop::ToJS(env, event)};
    Napi::Value result = dispatchEventFunc.Call(jsEventTarget, args);
    return result.As<Napi::Boolean>().Value();
}

}  // namespace wgpu::binding
