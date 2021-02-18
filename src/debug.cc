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

#include <stdio.h>

#include <sstream>
#include <string>
#include <vector>

#include "src/diagnostic/diagnostic.h"

namespace tint {
namespace {

InternalCompilerErrorReporter* ice_reporter = nullptr;

/// Note - this class is _not_ thread safe. If we have multiple internal
/// compiler errors occurring at the same time on different threads, then
/// we're in serious trouble.
class SourceFileToDelete {
  static SourceFileToDelete* instance;

 public:
  /// Adds file to the list that will be deleted on call to Free()
  static void Add(Source::File* file) {
    if (!instance) {
      instance = new SourceFileToDelete();
    }
    instance->files.emplace_back(file);
  }

  /// Free deletes all the source files added by calls to Add() and then this
  /// SourceFileToDelete object.
  static void Free() {
    if (instance) {
      for (auto* file : instance->files) {
        delete file;
      }
      delete instance;
      instance = nullptr;
    }
  }

 private:
  std::vector<Source::File*> files;
};

SourceFileToDelete* SourceFileToDelete::instance = nullptr;

}  // namespace

void FreeInternalCompilerErrors() {
  SourceFileToDelete::Free();
}

void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter) {
  ice_reporter = reporter;
}

InternalCompilerError::InternalCompilerError(const char* file,
                                             size_t line,
                                             diag::List& diagnostics)
    : file_(file), line_(line), diagnostics_(diagnostics) {}

InternalCompilerError::~InternalCompilerError() {
  auto* file = new Source::File(file_, "");

  SourceFileToDelete::Add(file);

  Source source{Source::Range{Source::Location{line_}}, file};
  diagnostics_.add_ice(msg_.str(), source);

  if (ice_reporter) {
    ice_reporter(diagnostics_);
  }
}

}  // namespace tint
