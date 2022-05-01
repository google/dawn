// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_UTILS_TERRIBLECOMMANDBUFFER_H_
#define SRC_DAWN_UTILS_TERRIBLECOMMANDBUFFER_H_

#include "dawn/wire/Wire.h"

namespace utils {

class TerribleCommandBuffer : public dawn::wire::CommandSerializer {
  public:
    TerribleCommandBuffer();
    explicit TerribleCommandBuffer(dawn::wire::CommandHandler* handler);

    void SetHandler(dawn::wire::CommandHandler* handler);

    size_t GetMaximumAllocationSize() const override;

    void* GetCmdSpace(size_t size) override;
    bool Flush() override;

  private:
    dawn::wire::CommandHandler* mHandler = nullptr;
    size_t mOffset = 0;
    char mBuffer[1000000];
};

}  // namespace utils

#endif  // SRC_DAWN_UTILS_TERRIBLECOMMANDBUFFER_H_
