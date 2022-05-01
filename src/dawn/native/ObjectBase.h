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

#ifndef SRC_DAWN_NATIVE_OBJECTBASE_H_
#define SRC_DAWN_NATIVE_OBJECTBASE_H_

#include <string>

#include "dawn/common/LinkedList.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/Forward.h"

namespace dawn::native {

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
    ~ApiObjectBase() override;

    virtual ObjectType GetType() const = 0;
    const std::string& GetLabel() const;

    // The ApiObjectBase is considered alive if it is tracked in a respective linked list owned
    // by the owning device.
    bool IsAlive() const;

    // This needs to be public because it can be called from the device owning the object.
    void Destroy();

    // Dawn API
    void APISetLabel(const char* label);

  protected:
    // Overriding of the RefCounted's DeleteThis function ensures that instances of objects
    // always call their derived class implementation of Destroy prior to the derived
    // class being destroyed. This guarantees that when ApiObjects' reference counts drop to 0,
    // then the underlying backend's Destroy calls are executed. We cannot naively put the call
    // to Destroy in the destructor of this class because it calls DestroyImpl
    // which is a virtual function often implemented in the Derived class which would already
    // have been destroyed by the time ApiObject's destructor is called by C++'s destruction
    // order. Note that some classes like BindGroup may override the DeleteThis function again,
    // and they should ensure that their overriding versions call this underlying version
    // somewhere.
    void DeleteThis() override;
    void TrackInDevice();

    // Sub-classes may override this function multiple times. Whenever overriding this function,
    // however, users should be sure to call their parent's version in the new override to make
    // sure that all destroy functionality is kept. This function is guaranteed to only be
    // called once through the exposed Destroy function.
    virtual void DestroyImpl() = 0;

  private:
    virtual void SetLabelImpl();

    std::string mLabel;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_OBJECTBASE_H_
