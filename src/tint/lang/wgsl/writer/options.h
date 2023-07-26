// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_WGSL_WRITER_OPTIONS_H_
#define SRC_TINT_LANG_WGSL_WRITER_OPTIONS_H_

#include "src/tint/utils/reflection/reflection.h"

namespace tint::wgsl::writer {

/// Configuration options used for generating WGSL.
struct Options {
#ifdef TINT_BUILD_SYNTAX_TREE_WRITER
    /// Constructor
    Options();
    /// Destructor
    ~Options();
    /// Copy constructor
    Options(const Options&);
    /// Copy assignment
    /// @returns this Options
    Options& operator=(const Options&);

    /// Set to `true` to use the syntax tree writer
    bool use_syntax_tree_writer = false;

    TINT_REFLECT(use_syntax_tree_writer);
#endif
};

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_OPTIONS_H_
