// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_FUZZERS_DAWNLPMFUZZER_H_
#define SRC_DAWN_FUZZERS_DAWNLPMFUZZER_H_

#include "dawn/fuzzers/lpmfuzz/dawn_lpm_autogen.pb.h"
#include "dawn/webgpu_cpp.h"

namespace dawn::native {

class Adapter;

}  // namespace dawn::native

namespace DawnLPMFuzzer {

int Initialize(int* argc, char*** argv);

int Run(const fuzzing::Program& program, bool (*AdapterSupported)(const dawn::native::Adapter&));
}  // namespace DawnLPMFuzzer

#endif  // SRC_DAWN_FUZZERS_DAWNLPMFUZZER_H_
