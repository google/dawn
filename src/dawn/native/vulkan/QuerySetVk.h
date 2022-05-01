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

#ifndef SRC_DAWN_NATIVE_VULKAN_QUERYSETVK_H_
#define SRC_DAWN_NATIVE_VULKAN_QUERYSETVK_H_

#include "dawn/native/QuerySet.h"

#include "dawn/common/vulkan_platform.h"

namespace dawn::native::vulkan {

class Device;

class QuerySet final : public QuerySetBase {
  public:
    static ResultOrError<Ref<QuerySet>> Create(Device* device,
                                               const QuerySetDescriptor* descriptor);

    VkQueryPool GetHandle() const;

  private:
    ~QuerySet() override;
    using QuerySetBase::QuerySetBase;
    MaybeError Initialize();

    // Dawn API
    void DestroyImpl() override;
    void SetLabelImpl() override;

    VkQueryPool mHandle = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_QUERYSETVK_H_
