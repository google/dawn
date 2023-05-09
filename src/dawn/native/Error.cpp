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

#include "dawn/native/Error.h"

#include "dawn/native/ErrorData.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

void IgnoreErrors(MaybeError maybeError) {
    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> errorData = maybeError.AcquireError();
        // During shutdown and destruction, device lost errors can be ignored.
        // We can also ignore other unexpected internal errors on shut down and treat it as
        // device lost so that we can continue with destruction.
        ASSERT(errorData->GetType() == InternalErrorType::DeviceLost ||
               errorData->GetType() == InternalErrorType::Internal);
    }
}

wgpu::ErrorType ToWGPUErrorType(InternalErrorType type) {
    switch (type) {
        case InternalErrorType::Validation:
            return wgpu::ErrorType::Validation;
        case InternalErrorType::OutOfMemory:
            return wgpu::ErrorType::OutOfMemory;
        case InternalErrorType::Internal:
            return wgpu::ErrorType::Internal;
        case InternalErrorType::DeviceLost:
            return wgpu::ErrorType::DeviceLost;

        default:
            return wgpu::ErrorType::Unknown;
    }
}

InternalErrorType FromWGPUErrorType(wgpu::ErrorType type) {
    switch (type) {
        case wgpu::ErrorType::Validation:
            return InternalErrorType::Validation;
        case wgpu::ErrorType::OutOfMemory:
            return InternalErrorType::OutOfMemory;
        case wgpu::ErrorType::DeviceLost:
            return InternalErrorType::DeviceLost;
        default:
            return InternalErrorType::Internal;
    }
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString |
                          absl::FormatConversionCharSet::kIntegral>
AbslFormatConvert(InternalErrorType value,
                  const absl::FormatConversionSpec& spec,
                  absl::FormatSink* s) {
    if (spec.conversion_char() == absl::FormatConversionChar::s) {
        if (!static_cast<bool>(value)) {
            s->Append("None");
            return {true};
        }

        bool moreThanOneBit = !HasZeroOrOneBits(value);
        if (moreThanOneBit) {
            s->Append("(");
        }

        bool first = true;
        if (value & InternalErrorType::Validation) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("Validation");
            value &= ~InternalErrorType::Validation;
        }
        if (value & InternalErrorType::DeviceLost) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("DeviceLost");
            value &= ~InternalErrorType::DeviceLost;
        }
        if (value & InternalErrorType::Internal) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("Internal");
            value &= ~InternalErrorType::Internal;
        }
        if (value & InternalErrorType::OutOfMemory) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("OutOfMemory");
            value &= ~InternalErrorType::OutOfMemory;
        }

        if (moreThanOneBit) {
            s->Append(")");
        }
    } else {
        s->Append(absl::StrFormat(
            "%u", static_cast<typename std::underlying_type<InternalErrorType>::type>(value)));
    }
    return {true};
}

}  // namespace dawn::native
