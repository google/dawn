// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D_TEXTURED3D_H_
#define SRC_DAWN_NATIVE_D3D_TEXTURED3D_H_

#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Texture.h"

namespace dawn::native::d3d {

class Texture : public TextureBase {
  public:
    virtual ResultOrError<ExecutionSerial> EndAccess() = 0;

  protected:
    using TextureBase::TextureBase;
    ~Texture() override;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_TEXTURED3D_H_
