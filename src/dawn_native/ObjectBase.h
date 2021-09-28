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

#include "common/LinkedList.h"
#include "common/RefCounted.h"
#include "dawn_native/Forward.h"

#include <string>

namespace dawn_native {

    class DeviceBase;

    class ObjectBase : public RefCounted {
      public:
        struct ErrorTag {};
        static constexpr ErrorTag kError = {};

        explicit ObjectBase(DeviceBase* device);
        ObjectBase(DeviceBase* device, ErrorTag tag);

        DeviceBase* GetDevice() const;
        bool IsError() const;
        bool IsAlive() const;
        void DestroyObject();

      private:
        // Pointer to owning device, if nullptr, that means that the object is no longer alive or
        // valid.
        DeviceBase* mDevice;
    };

    class ApiObjectBase : public ObjectBase, public LinkNode<ApiObjectBase> {
      public:
        struct LabelNotImplementedTag {};
        static constexpr LabelNotImplementedTag kLabelNotImplemented = {};

        ApiObjectBase(DeviceBase* device, LabelNotImplementedTag tag);
        ApiObjectBase(DeviceBase* device, const char* label);
        ApiObjectBase(DeviceBase* device, ErrorTag tag);

        virtual ObjectType GetType() const = 0;
        const std::string& GetLabel() const;

        // Dawn API
        void APISetLabel(const char* label);

      private:
        virtual void SetLabelImpl();

        std::string mLabel;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_OBJECTBASE_H_
