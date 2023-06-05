// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_UTILS_OBJCUTILS_H_
#define SRC_DAWN_UTILS_OBJCUTILS_H_

#include "dawn/common/NonCopyable.h"

// Contains helper function to manipulate ObjC objects. This helps having C++ files do a little bit
// of ObjectiveC calls, when they cannot be converted to ObjectiveC++ because they are used on
// multiple platforms.

namespace dawn::utils {

// The returned CALayer is autoreleased.
class ScopedCALayer : public NonMovable {
  public:
    explicit ScopedCALayer(void* layer);
    ~ScopedCALayer();

    void* const layer;
};
ScopedCALayer CreatePlaceholderCALayer();

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_OBJCUTILS_H_
