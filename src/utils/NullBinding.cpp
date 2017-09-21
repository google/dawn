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

#include "utils/BackendBinding.h"

namespace backend {
    namespace null {
        void Init(nxtProcTable* procs, nxtDevice* device);
    }
}

namespace utils {

    class NullBinding : public BackendBinding {
        public:
            void SetupGLFWWindowHints() override {
            }
            void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
                backend::null::Init(procs, device);
            }
            uint64_t GetSwapChainImplementation() override {
                return 0;
            }
            nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
                return NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
            }
    };


    BackendBinding* CreateNullBinding() {
        return new NullBinding;
    }

}
