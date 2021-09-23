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

#ifndef DAWNNATIVE_OBJECTBASE_H_
#define DAWNNATIVE_OBJECTBASE_H_

#include "common/RefCounted.h"

#include <string>

namespace dawn_native {

    class DeviceBase;

    class ObjectBase : public RefCounted {
      public:
        struct ErrorTag {};
        static constexpr ErrorTag kError = {};
        struct LabelNotImplementedTag {};
        static constexpr LabelNotImplementedTag kLabelNotImplemented = {};

        ObjectBase(DeviceBase* device, LabelNotImplementedTag tag);
        ObjectBase(DeviceBase* device, const char* label);
        ObjectBase(DeviceBase* device, ErrorTag tag);

        DeviceBase* GetDevice() const;
        const std::string& GetLabel() const;
        bool IsError() const;

        // Dawn API
        void APISetLabel(const char* label);

      private:
        virtual void SetLabelImpl();

        // TODO(dawn:840): Optimize memory footprint for objects that don't have labels.
        std::string mLabel;
        DeviceBase* mDevice;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_OBJECTBASE_H_
