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

#ifndef SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREFENCEMTL_H_
#define SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREFENCEMTL_H_

#include <os/availability.h>
#include <vector>

#include "dawn/common/NSRef.h"
#include "dawn/native/Error.h"
#include "dawn/native/SharedFence.h"

@protocol MTLSharedEvent;

namespace dawn::native::metal {

class Device;

class API_AVAILABLE(macos(10.14), ios(12.0)) SharedFence final : public SharedFenceBase {
  public:
    static ResultOrError<Ref<SharedFence>> Create(
        Device* device,
        const char* label,
        const SharedFenceMTLSharedEventDescriptor* descriptor);

    id<MTLSharedEvent> GetMTLSharedEvent() const;

  private:
    SharedFence(Device* device, const char* label, id<MTLSharedEvent> sharedEvent);

    MaybeError ExportInfoImpl(SharedFenceExportInfo* info) const override;

    NSPRef<id<MTLSharedEvent>> mSharedEvent;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREFENCEMTL_H_
