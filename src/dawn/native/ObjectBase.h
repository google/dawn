// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_OBJECTBASE_H_
#define SRC_DAWN_NATIVE_OBJECTBASE_H_

#include <mutex>
#include <string>

#include "dawn/common/LinkedList.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/Forward.h"

namespace absl {
class FormatSink;
}

namespace dawn::native {

class ApiObjectBase;
class DeviceBase;

class ErrorMonad : public RefCounted {
  public:
    struct ErrorTag {};
    static constexpr ErrorTag kError = {};

    ErrorMonad();
    explicit ErrorMonad(ErrorTag tag);

    bool IsError() const;
};

class ObjectBase : public ErrorMonad {
  public:
    explicit ObjectBase(DeviceBase* device);
    ObjectBase(DeviceBase* device, ErrorTag tag);

    InstanceBase* GetInstance() const;
    DeviceBase* GetDevice() const;

  private:
    // Ref to owning device.
    Ref<DeviceBase> mDevice;
};

// Generic object list with a mutex for tracking for destruction.
class ApiObjectList {
  public:
    // Tracks an object if the list is not destroyed. If the list is destroyed, destroys the object.
    void Track(ApiObjectBase* object);

    // Returns true iff the object was removed from the list.
    bool Untrack(ApiObjectBase* object);

    // Destroys and removes all the objects tracked in the list.
    void Destroy();

  private:
    // Boolean used to mark the list so that on subsequent calls to Untrack, we don't need to
    // reaquire the lock, and Track on new objects immediately destroys them.
    bool mMarkedDestroyed = false;
    std::mutex mMutex;
    LinkedList<ApiObjectBase> mObjects;
};

class ApiObjectBase : public ObjectBase, public LinkNode<ApiObjectBase> {
  public:
    struct LabelNotImplementedTag {};
    static constexpr LabelNotImplementedTag kLabelNotImplemented = {};
    struct UntrackedByDeviceTag {};
    static constexpr UntrackedByDeviceTag kUntrackedByDevice = {};

    ApiObjectBase(DeviceBase* device, LabelNotImplementedTag tag);
    ApiObjectBase(DeviceBase* device, const char* label);
    ApiObjectBase(DeviceBase* device, ErrorTag tag, const char* label = nullptr);
    ~ApiObjectBase() override;

    virtual ObjectType GetType() const = 0;
    void SetLabel(std::string label);
    const std::string& GetLabel() const;

    virtual void FormatLabel(absl::FormatSink* s) const;

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
    void LockAndDeleteThis() override;

    // Returns the list where this object may be tracked for future destruction. This can be
    // overrided to create hierarchical object tracking ownership:
    //   i.e. Device -[tracks]-> Texture -[tracks]-> TextureView.
    virtual ApiObjectList* GetObjectTrackingList();

    // Sub-classes may override this function multiple times. Whenever overriding this function,
    // however, users should be sure to call their parent's version in the new override to make
    // sure that all destroy functionality is kept. This function is guaranteed to only be
    // called once through the exposed Destroy function.
    virtual void DestroyImpl() = 0;

  private:
    friend class ApiObjectList;

    virtual void SetLabelImpl();

    std::string mLabel;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_OBJECTBASE_H_
