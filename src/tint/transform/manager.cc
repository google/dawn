// Copyright 2020 The Tint Authors.
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

#include "src/tint/transform/manager.h"

/// If set to 1 then the transform::Manager will dump the WGSL of the program
/// before and after each transform. Helpful for debugging bad output.
#define TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM 0

#if TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
#define TINT_IF_PRINT_PROGRAM(x) x
#else  // TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
#define TINT_IF_PRINT_PROGRAM(x)
#endif  // TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM

TINT_INSTANTIATE_TYPEINFO(tint::transform::Manager);

namespace tint::transform {

Manager::Manager() = default;
Manager::~Manager() = default;

Output Manager::Run(const Program* program, const DataMap& data) const {
    const Program* in = program;

#if TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
    auto print_program = [&](const char* msg, const Transform* transform) {
        auto wgsl = Program::printer(in);
        std::cout << "---------------------------------------------------------" << std::endl;
        std::cout << "-- " << msg << " " << transform->TypeInfo().name << ":" << std::endl;
        std::cout << "---------------------------------------------------------" << std::endl;
        std::cout << wgsl << std::endl;
        std::cout << "---------------------------------------------------------" << std::endl
                  << std::endl;
    };
#endif

    Output out;
    for (const auto& transform : transforms_) {
        if (!transform->ShouldRun(in, data)) {
            TINT_IF_PRINT_PROGRAM(std::cout << "Skipping " << transform->TypeInfo().name);
            continue;
        }
        TINT_IF_PRINT_PROGRAM(print_program("Input to", transform.get()));

        auto res = transform->Run(in, data);
        out.program = std::move(res.program);
        out.data.Add(std::move(res.data));
        in = &out.program;
        if (!in->IsValid()) {
            TINT_IF_PRINT_PROGRAM(print_program("Invalid output of", transform.get()));
            return out;
        }

        if (transform == transforms_.back()) {
            TINT_IF_PRINT_PROGRAM(print_program("Output of", transform.get()));
        }
    }

    if (program == in) {
        out.program = program->Clone();
    }

    return out;
}

}  // namespace tint::transform
