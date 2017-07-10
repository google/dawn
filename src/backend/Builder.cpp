// Copyright 2017 The NXT Authors
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

#include "backend/Builder.h"

#include "backend/Device.h"
#include "common/Assert.h"

namespace backend {

    bool BuilderBase::CanBeUsed() const {
        return !consumed && !gotStatus;
    }

    DeviceBase* BuilderBase::GetDevice() {
        return device;
    }

    void BuilderBase::HandleError(const char* message) {
        SetStatus(nxt::BuilderErrorStatus::Error, message);
    }

    void BuilderBase::SetErrorCallback(nxt::BuilderErrorCallback callback,
                                   nxt::CallbackUserdata userdata1,
                                   nxt::CallbackUserdata userdata2) {
        this->callback = callback;
        this->userdata1 = userdata1;
        this->userdata2 = userdata2;
    }

    BuilderBase::BuilderBase(DeviceBase* device) : device(device) {
    }

    BuilderBase::~BuilderBase() {
        if (!consumed && callback != nullptr) {
            callback(NXT_BUILDER_ERROR_STATUS_UNKNOWN, "Builder destroyed before GetResult", userdata1, userdata2);
        }
    }

    void BuilderBase::SetStatus(nxt::BuilderErrorStatus status, const char* message) {
        ASSERT(status != nxt::BuilderErrorStatus::Success);
        ASSERT(status != nxt::BuilderErrorStatus::Unknown);
        ASSERT(!gotStatus); // This is not strictly necessary but something to strive for.
        gotStatus = true;

        storedStatus = status;
        storedMessage = message;
    }

    bool BuilderBase::HandleResult(RefCounted* result) {
        // GetResult can only be called once.
        ASSERT(!consumed);
        consumed = true;

        // result == nullptr implies there was an error which implies we should have a status set.
        ASSERT(result != nullptr || gotStatus);

        // If we have any error, then we have to return nullptr
        if (gotStatus) {
            ASSERT(storedStatus != nxt::BuilderErrorStatus::Success);

            // The application will never see "result" so we need to remove the
            // external ref here.
            if (result != nullptr) {
                result->Release();
                result = nullptr;
            }
        } else {
            ASSERT(storedStatus == nxt::BuilderErrorStatus::Success);
            ASSERT(storedMessage.empty());
        }

        if (callback) {
            callback(static_cast<nxtBuilderErrorStatus>(storedStatus), storedMessage.c_str(), userdata1, userdata2);
        }

        return result != nullptr;
    }

}
