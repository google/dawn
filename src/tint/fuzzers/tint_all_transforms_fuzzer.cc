// Copyright 2021 The Tint Authors.
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

#include "src/tint/fuzzers/fuzzer_init.h"
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/transform_builder.h"

namespace tint::fuzzers {

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kSpv);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }

#if TINT_BUILD_HLSL_WRITER
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kHLSL);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_MSL_WRITER
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kMSL);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }
#endif  // TINT_BUILD_MSL_WRITER
#if TINT_BUILD_SPV_WRITER
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kSpv);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }
#endif  // TINT_BUILD_SPV_WRITER

    return 0;
}

}  // namespace tint::fuzzers
