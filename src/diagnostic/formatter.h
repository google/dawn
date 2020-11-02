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

#ifndef SRC_DIAGNOSTIC_FORMATTER_H_
#define SRC_DIAGNOSTIC_FORMATTER_H_

#include <memory>
#include <string>

namespace tint {
namespace diag {

class List;

/// Formatters are used to format a list of diagnostics into a human readable
/// string.
class Formatter {
 public:
  /// @returns a basic diagnostic formatter
  /// @param print_file include the file path for each diagnostic
  /// @param print_severity include the severity for each diagnostic
  /// @param print_line include the source line(s) for the diagnostic
  static std::unique_ptr<Formatter> create(bool print_file,
                                           bool print_severity,
                                           bool print_line);

  virtual ~Formatter();

  /// @return the human readable list of diagnostics formatted to a string.
  virtual std::string format(const List&) const = 0;
};

}  // namespace diag
}  // namespace tint

#endif  // SRC_DIAGNOSTIC_FORMATTER_H_
