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

#ifndef SRC_TINT_WRITER_TEXT_H_
#define SRC_TINT_WRITER_TEXT_H_

#include <string>

#include "src/tint/writer/writer.h"

namespace tint::writer {

/// Class to generate text source
class Text : public Writer {
  public:
    ~Text() override;

    /// @returns the result data
    virtual std::string result() const = 0;
};

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_TEXT_H_
