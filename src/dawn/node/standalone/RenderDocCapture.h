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

#ifndef SRC_DAWN_NODE_STANDALONE_RENDERDOCCAPTURE_H_
#define SRC_DAWN_NODE_STANDALONE_RENDERDOCCAPTURE_H_

#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/interop/NodeAPI.h"

namespace dawn::node::standalone {

// Captures each CTS test case in RenderDoc.
class RenderDocCapture {
  public:
    // Sets the CTS's test case callbacks so that each test case is captured in a RenderDoc frame
    // capture, and makes the CTS keep at most one device.
    void Configure(Napi::Env env);

  private:
    // Starts a RenderDoc frame capture for `info[0]` (a JS GPUDevice), using the `RenderDocCapture`
    // in `info.Data()`. Returns whether a capture started.
    static Napi::Value StartCapture(const Napi::CallbackInfo& info);

    // Ends the active RenderDoc frame capture and stores `info[0]` (the test case) in its comments,
    // using the `RenderDocCapture` in `info.Data()`.
    static void EndCapture(const Napi::CallbackInfo& info);

    // The device of the capture in progress.
    wgpu::Device capture_device_;
};

}  // namespace dawn::node::standalone

#endif  // SRC_DAWN_NODE_STANDALONE_RENDERDOCCAPTURE_H_
