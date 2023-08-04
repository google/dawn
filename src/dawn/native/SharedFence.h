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

#ifndef SRC_DAWN_NATIVE_SHAREDFENCE_H_
#define SRC_DAWN_NATIVE_SHAREDFENCE_H_

#include "dawn/native/ObjectBase.h"

namespace dawn::native {

struct SharedFenceDescriptor;
struct SharedFenceExportInfo;

class SharedFenceBase : public ApiObjectBase {
  public:
    static SharedFenceBase* MakeError(DeviceBase* device, const SharedFenceDescriptor* descriptor);

    ObjectType GetType() const override;

    void APIExportInfo(SharedFenceExportInfo* info) const;

  private:
    void DestroyImpl() override;

    SharedFenceBase(DeviceBase* device,
                    const SharedFenceDescriptor* descriptor,
                    ObjectBase::ErrorTag tag);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDFENCE_H_
