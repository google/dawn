// Copyright 2021 The Dawn Authors
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

#include "dawn/native/TintUtils.h"

#include "dawn/native/Device.h"

#include "tint/tint.h"

namespace dawn::native {

namespace {

thread_local DeviceBase* tlDevice = nullptr;

void TintICEReporter(const tint::diag::List& diagnostics) {
    if (tlDevice) {
        tlDevice->HandleError(InternalErrorType::Validation, diagnostics.str().c_str());
    }
}

bool InitializeTintErrorReporter() {
    tint::SetInternalCompilerErrorReporter(&TintICEReporter);
    return true;
}

}  // namespace

ScopedTintICEHandler::ScopedTintICEHandler(DeviceBase* device) {
    // Call tint::SetInternalCompilerErrorReporter() the first time
    // this constructor is called. Static initialization is
    // guaranteed to be thread-safe, and only occur once.
    static bool init_once_tint_error_reporter = InitializeTintErrorReporter();
    (void)init_once_tint_error_reporter;

    // Shouldn't have overlapping instances of this handler.
    ASSERT(tlDevice == nullptr);
    tlDevice = device;
}

ScopedTintICEHandler::~ScopedTintICEHandler() {
    tlDevice = nullptr;
}

}  // namespace dawn::native
