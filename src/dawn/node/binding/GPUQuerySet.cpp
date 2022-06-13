// Copyright 2021 The Dawn Authors
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

#include "src/dawn/node/binding/GPUQuerySet.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUQuerySet
////////////////////////////////////////////////////////////////////////////////
GPUQuerySet::GPUQuerySet(wgpu::QuerySet query_set) : query_set_(std::move(query_set)) {}

void GPUQuerySet::destroy(Napi::Env) {
    query_set_.Destroy();
}

interop::GPUQueryType GPUQuerySet::getType(Napi::Env env) {
    interop::GPUQueryType result;

    Converter conv(env);
    if (!conv(result, query_set_.GetType())) {
        Napi::Error::New(env, "Couldn't convert type to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUQueryType::kOcclusion;  // Doesn't get used.
    }

    return result;
}

interop::GPUSize32 GPUQuerySet::getCount(Napi::Env) {
    return query_set_.GetCount();
}

std::string GPUQuerySet::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPUQuerySet::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
