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

#ifndef SRC_DAWN_NATIVE_METAL_QUERYSETMTL_H_
#define SRC_DAWN_NATIVE_METAL_QUERYSETMTL_H_

#include "dawn/native/QuerySet.h"

#include "dawn/common/NSRef.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class Device;

class QuerySet final : public QuerySetBase {
  public:
    static ResultOrError<Ref<QuerySet>> Create(Device* device,
                                               const QuerySetDescriptor* descriptor);

    QuerySet(DeviceBase* device, const QuerySetDescriptor* descriptor);

    id<MTLBuffer> GetVisibilityBuffer() const;
    id<MTLCounterSampleBuffer> GetCounterSampleBuffer() const
        API_AVAILABLE(macos(10.15), ios(14.0));

  private:
    using QuerySetBase::QuerySetBase;
    MaybeError Initialize();

    ~QuerySet() override;

    // Dawn API
    void DestroyImpl() override;

    NSPRef<id<MTLBuffer>> mVisibilityBuffer;
    // Note that mCounterSampleBuffer cannot be an NSRef because the API_AVAILABLE macros don't
    // propagate nicely through templates.
    id<MTLCounterSampleBuffer> mCounterSampleBuffer API_AVAILABLE(macos(10.15),
                                                                  ios(14.0)) = nullptr;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_QUERYSETMTL_H_
