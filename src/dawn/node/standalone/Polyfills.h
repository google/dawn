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

#ifndef SRC_DAWN_NODE_STANDALONE_POLYFILLS_H_
#define SRC_DAWN_NODE_STANDALONE_POLYFILLS_H_

#include <functional>
#include <string>
#include <vector>

#include "src/dawn/node/interop/NodeAPI.h"

namespace dawn::node::standalone {

// Configuration options for polyfill registration.
struct PolyfillOptions {
    // Values reported by `process.argv`.
    std::vector<std::string> argv;

    // Invoked by `process.exit()`. When unset, `process.exit()` calls std::exit() directly.
    std::function<void(int32_t)> on_exit;
};

// Registers Node.js and Web standard polyfills (console, process, fs, path, timers, DOM stubs)
// into the provided environment's global scope.
void RegisterPolyfills(Napi::Env env, const PolyfillOptions& options = {});

}  // namespace dawn::node::standalone

#endif  // SRC_DAWN_NODE_STANDALONE_POLYFILLS_H_
