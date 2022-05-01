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

#include "src/tint/debug.h"

#include <memory>

#include "src/tint/utils/debugger.h"

namespace tint {
namespace {

InternalCompilerErrorReporter* ice_reporter = nullptr;

}  // namespace

void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter) {
    ice_reporter = reporter;
}

InternalCompilerError::InternalCompilerError(const char* file,
                                             size_t line,
                                             diag::System system,
                                             diag::List& diagnostics)
    : file_(file), line_(line), system_(system), diagnostics_(diagnostics) {}

InternalCompilerError::~InternalCompilerError() {
    auto file = std::make_shared<Source::File>(file_, "");
    Source source{Source::Range{{line_}}, file.get()};
    diagnostics_.add_ice(system_, msg_.str(), source, std::move(file));

    if (ice_reporter) {
        ice_reporter(diagnostics_);
    }

    debugger::Break();
}

}  // namespace tint
