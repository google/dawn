// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_PROGRAMMABLEENCODER_H_
#define SRC_DAWN_NATIVE_PROGRAMMABLEENCODER_H_

#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

// Base class for shared functionality between programmable encoders.
class ProgrammableEncoder : public ApiObjectBase {
  public:
    ProgrammableEncoder(DeviceBase* device, const char* label, EncodingContext* encodingContext);

    void APIInsertDebugMarker(const char* groupLabel);
    void APIPopDebugGroup();
    void APIPushDebugGroup(const char* groupLabel);

  protected:
    bool IsValidationEnabled() const;
    MaybeError ValidateProgrammableEncoderEnd() const;

    // Compute and render passes do different things on SetBindGroup. These are helper functions
    // for the logic they have in common.
    MaybeError ValidateSetBindGroup(BindGroupIndex index,
                                    BindGroupBase* group,
                                    uint32_t dynamicOffsetCountIn,
                                    const uint32_t* dynamicOffsetsIn) const;
    void RecordSetBindGroup(CommandAllocator* allocator,
                            BindGroupIndex index,
                            BindGroupBase* group,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets) const;

    // Construct an "error" programmable pass encoder.
    ProgrammableEncoder(DeviceBase* device, EncodingContext* encodingContext, ErrorTag errorTag);

    EncodingContext* mEncodingContext = nullptr;

    uint64_t mDebugGroupStackSize = 0;

    bool mEnded = false;

  private:
    const bool mValidationEnabled;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PROGRAMMABLEENCODER_H_
