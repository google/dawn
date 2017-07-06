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

#include "backend/RefCounted.h"

#include <cassert>
#define ASSERT assert

namespace backend {

    RefCounted::RefCounted() {
    }

    RefCounted::~RefCounted() {
    }

    void RefCounted::ReferenceInternal() {
        ASSERT(internalRefs != 0);
        // TODO(cwallez@chromium.org): what to do on overflow?
        internalRefs ++;
    }

    void RefCounted::ReleaseInternal() {
        ASSERT(internalRefs != 0);
        internalRefs --;
        if (internalRefs == 0) {
            ASSERT(externalRefs == 0);
            // TODO(cwallez@chromium.org): would this work with custom allocators?
            delete this;
        }
    }

    uint32_t RefCounted::GetExternalRefs() const {
        return externalRefs;
    }

    uint32_t RefCounted::GetInternalRefs() const {
        return internalRefs;
    }

    void RefCounted::Reference() {
        ASSERT(externalRefs != 0);
        // TODO(cwallez@chromium.org): what to do on overflow?
        externalRefs ++;
    }

    void RefCounted::Release() {
        ASSERT(externalRefs != 0);
        externalRefs --;
        if (externalRefs == 0) {
            ReleaseInternal();
        }
    }

}
