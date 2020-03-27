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

#ifndef SRC_READER_READER_H_
#define SRC_READER_READER_H_

#include <string>

#include "src/ast/module.h"
#include "src/context.h"

namespace tint {
namespace reader {

/// Base class for input readers
class Reader {
 public:
  virtual ~Reader();

  /// Parses the input data
  /// @returns true if the parse was successful
  virtual bool Parse() = 0;

  /// @returns true if an error was encountered
  bool has_error() const { return error_.size() > 0; }
  /// @returns the parser error string
  const std::string& error() const { return error_; }

  /// @returns the module. The module in the parser will be reset after this.
  virtual ast::Module module() = 0;

 protected:
  /// Constructor
  /// @param ctx the context object, must be non-null
  explicit Reader(Context* ctx);

  /// Sets the error string
  /// @param msg the error message
  void set_error(const std::string& msg) { error_ = msg; }

  /// The Tint context object
  Context& ctx_;

  /// An error message, if an error was encountered
  std::string error_;
};

}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_READER_H_
