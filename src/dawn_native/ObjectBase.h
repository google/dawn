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

      private:
        // Pointer to owning device.
        DeviceBase* mDevice;
    };

    class ApiObjectBase : public ObjectBase, public LinkNode<ApiObjectBase> {
      public:
        struct LabelNotImplementedTag {};
        static constexpr LabelNotImplementedTag kLabelNotImplemented = {};
        struct UntrackedByDeviceTag {};
        static constexpr UntrackedByDeviceTag kUntrackedByDevice = {};

        ApiObjectBase(DeviceBase* device, LabelNotImplementedTag tag);
        ApiObjectBase(DeviceBase* device, const char* label);
        ApiObjectBase(DeviceBase* device, ErrorTag tag);
        virtual ~ApiObjectBase() override;

        virtual ObjectType GetType() const = 0;
        const std::string& GetLabel() const;

        // The ApiObjectBase is considered alive if it is tracked in a respective linked list owned
        // by the owning device.
        bool IsAlive() const;

        // Allow virtual overriding of actual destroy call in order to allow for re-using of base
        // destruction oerations. Classes that override this function should almost always call this
        // class's implementation in the override. This needs to be public because it can be called
        // from the device owning the object. Returns true iff destruction occurs. Upon any re-calls
        // of the function it will return false to indicate no further operations should be taken.
        virtual bool DestroyApiObject();

        // Dawn API
        void APISetLabel(const char* label);

      protected:
        void TrackInDevice();
        virtual void DestroyApiObjectImpl();

      private:
        virtual void SetLabelImpl();

        std::string mLabel;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_OBJECTBASE_H_
