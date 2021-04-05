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

#ifndef DAWNWIRE_WIRERESULT_H_
#define DAWNWIRE_WIRERESULT_H_

#include "common/Compiler.h"

namespace dawn_wire {

    enum class DAWN_NO_DISCARD WireResult {
        Success,
        FatalError,
    };

// Macro to simplify error handling, similar to DAWN_TRY but for WireResult.
#define WIRE_TRY(EXPR)                                          \
    do {                                                        \
        WireResult exprResult = EXPR;                           \
        if (DAWN_UNLIKELY(exprResult != WireResult::Success)) { \
            return exprResult;                                  \
        }                                                       \
    } while (0)

}  // namespace dawn_wire

#endif  // DAWNWIRE_WIRERESULT_H_
