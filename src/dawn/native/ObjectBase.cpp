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

#include <mutex>
#include <utility>

#include "absl/strings/str_format.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"

namespace dawn::native {

static constexpr uint64_t kErrorPayload = 0;
static constexpr uint64_t kNotErrorPayload = 1;

ErrorMonad::ErrorMonad() : RefCounted(kNotErrorPayload) {}
ErrorMonad::ErrorMonad(ErrorTag) : RefCounted(kErrorPayload) {}

bool ErrorMonad::IsError() const {
    return GetRefCountPayload() == kErrorPayload;
}

ObjectBase::ObjectBase(DeviceBase* device) : ErrorMonad(), mDevice(device) {}

ObjectBase::ObjectBase(DeviceBase* device, ErrorTag) : ErrorMonad(kError), mDevice(device) {}

DeviceBase* ObjectBase::GetDevice() const {
    return mDevice.Get();
}

void ApiObjectList::Track(ApiObjectBase* object) {
    if (mMarkedDestroyed) {
        object->DestroyImpl();
        return;
    }
    std::lock_guard<std::mutex> lock(mMutex);
    mObjects.Prepend(object);
}

bool ApiObjectList::Untrack(ApiObjectBase* object) {
    std::lock_guard<std::mutex> lock(mMutex);
    return object->RemoveFromList();
}

void ApiObjectList::Destroy() {
    std::lock_guard<std::mutex> lock(mMutex);
    mMarkedDestroyed = true;
    while (!mObjects.empty()) {
        auto* head = mObjects.head();
        bool removed = head->RemoveFromList();
        ASSERT(removed);
        head->value()->DestroyImpl();
    }
}

ApiObjectBase::ApiObjectBase(DeviceBase* device, const char* label) : ObjectBase(device) {
    if (label) {
        mLabel = label;
    }
}

ApiObjectBase::ApiObjectBase(DeviceBase* device, ErrorTag tag, const char* label)
    : ObjectBase(device, tag) {
    if (label) {
        mLabel = label;
    }
}

ApiObjectBase::ApiObjectBase(DeviceBase* device, LabelNotImplementedTag tag) : ObjectBase(device) {}

ApiObjectBase::~ApiObjectBase() {
    ASSERT(!IsAlive());
}

void ApiObjectBase::APISetLabel(const char* label) {
    SetLabel(label);
}

void ApiObjectBase::APIRelease() {
    // TODO(crbug.com/dawn/1769): We have to lock the entire APIRelease() method.
    // This is because some objects are cached as raw pointers by the device. And the cache lookup
    // would have been racing with the ref count's decrement here if there had not been any locking
    // in place. This is temporary solution until we improve the cache's implementation.
    auto deviceLock(GetDevice()->GetScopedLockSafeForDelete());
    Release();
}

void ApiObjectBase::SetLabel(std::string label) {
    mLabel = std::move(label);
    SetLabelImpl();
}

const std::string& ApiObjectBase::GetLabel() const {
    return mLabel;
}

void ApiObjectBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));
    if (!mLabel.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", mLabel));
    }
}

void ApiObjectBase::SetLabelImpl() {}

bool ApiObjectBase::IsAlive() const {
    return IsInList();
}

void ApiObjectBase::DeleteThis() {
    Destroy();
    RefCounted::DeleteThis();
}

void ApiObjectBase::LockAndDeleteThis() {
    auto deviceLock(GetDevice()->GetScopedLockSafeForDelete());
    DeleteThis();
}

ApiObjectList* ApiObjectBase::GetObjectTrackingList() {
    ASSERT(GetDevice() != nullptr);
    return GetDevice()->GetObjectTrackingList(GetType());
}

void ApiObjectBase::Destroy() {
    if (!IsAlive()) {
        return;
    }
    ApiObjectList* list = GetObjectTrackingList();
    ASSERT(list != nullptr);
    if (list->Untrack(this)) {
        DestroyImpl();
    }
}

}  // namespace dawn::native
