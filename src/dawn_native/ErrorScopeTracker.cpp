// Copyright 2019 The Dawn Authors
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

#include "dawn_native/ErrorScopeTracker.h"

#include "dawn_native/Device.h"
#include "dawn_native/ErrorScope.h"

#include <limits>

namespace dawn_native {

    ErrorScopeTracker::ErrorScopeTracker(DeviceBase* device) : mDevice(device) {
    }

    ErrorScopeTracker::~ErrorScopeTracker() {
        ASSERT(mScopesInFlight.Empty());
    }

    void ErrorScopeTracker::TrackUntilLastSubmitComplete(ErrorScope* scope) {
        mScopesInFlight.Enqueue(scope, mDevice->GetLastSubmittedCommandSerial());
        mDevice->AddFutureSerial(mDevice->GetPendingCommandSerial());
    }

    void ErrorScopeTracker::Tick(ExecutionSerial completedSerial) {
        mScopesInFlight.ClearUpTo(completedSerial);
    }

    void ErrorScopeTracker::ClearForShutDown() {
        for (Ref<ErrorScope>& scope : mScopesInFlight.IterateAll()) {
            scope->UnlinkForShutdown();
        }
        mScopesInFlight.Clear();
    }

}  // namespace dawn_native
