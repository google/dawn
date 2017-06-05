// Copyright 2017 The NXT Authors
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

#include "BackendBinding.h"

namespace backend {
namespace d3d12 {
    void Init(nxtProcTable* procs, nxtDevice* device);
}
}

class D3D12Binding : public BackendBinding {
    public:
        void SetupGLFWWindowHints() override {
        }
        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            backend::d3d12::Init(procs, device);
        }
        void SwapBuffers() override {
        }
};

BackendBinding* CreateD3D12Binding() {
    return new D3D12Binding;
}
