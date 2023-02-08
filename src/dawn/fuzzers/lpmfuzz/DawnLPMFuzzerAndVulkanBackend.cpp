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

#include "dawn/common/GPUInfo.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMFuzzer.h"
#include "dawn/fuzzers/lpmfuzz/dawn_lpm_autogen.pb.h"
#include "dawn/native/DawnNative.h"
#include "testing/libfuzzer/libfuzzer_exports.h"
#include "testing/libfuzzer/proto/lpm_interface.h"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    return DawnLPMFuzzer::Initialize(argc, argv);
}

DEFINE_PROTO_FUZZER(const fuzzing::Program& program) {
    DawnLPMFuzzer::Run(program, [](const dawn::native::Adapter& adapter) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        return gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID);
    });
}
