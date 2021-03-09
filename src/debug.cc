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

#include "src/debug.h"

namespace tint {
namespace {

InternalCompilerErrorReporter* ice_reporter = nullptr;

}  // namespace

void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter) {
  ice_reporter = reporter;
}

InternalCompilerError::InternalCompilerError(const char* file,
                                             size_t line,
                                             diag::List& diagnostics)
    : file_(file), line_(line), diagnostics_(diagnostics) {}

InternalCompilerError::~InternalCompilerError() {
  Source source{Source::Range{Source::Location{line_}}, file_};
  diagnostics_.add_ice(msg_.str(), source);

  if (ice_reporter) {
    ice_reporter(diagnostics_);
  }
}

}  // namespace tint
