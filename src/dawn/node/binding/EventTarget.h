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

#ifndef SRC_DAWN_NODE_BINDING_EVENTTARGET_H_
#define SRC_DAWN_NODE_BINDING_EVENTTARGET_H_

#include <string>

#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// Holds a reference to a node native EventTarget and forward calls to these
// functions to that native object. We do this because GPUDevice is supposed
// to be an actual EventTarget. By wrapping we can make sure the behavior is
// the same. Ideally GPUDevice would inherit from EventTarget but that appears
// to be impossible using Napi.
class EventTarget : public virtual interop::EventTarget {
  public:
    explicit EventTarget(Napi::Env env);
    ~EventTarget();

    void addEventListener(
        Napi::Env,
        std::string type,
        std::optional<interop::EventListener> callback,
        std::optional<std::variant<interop::AddEventListenerOptions, bool>> options) override;
    void removeEventListener(
        Napi::Env,
        std::string type,
        std::optional<interop::EventListener> callback,
        std::optional<std::variant<interop::EventListenerOptions, bool>> options) override;
    bool dispatchEvent(Napi::Env, interop::Event event) override;

  private:
    // TODO(crbug.com/420932896): The Reference here is a GC root and would do
    // eventTargetRef->EventTarget->closure->JS
    // GPUDevice->binding::GPUDevice->bindings::EventTargetRef->eventTargetRef. which would prevent
    // cleanup.
    Napi::Reference<Napi::Object> eventTargetRef_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_EVENTTARGET_H_
