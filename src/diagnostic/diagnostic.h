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

#ifndef SRC_DIAGNOSTIC_DIAGNOSTIC_H_
#define SRC_DIAGNOSTIC_DIAGNOSTIC_H_

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "src/source.h"

namespace tint {
namespace diag {

/// Severity is an enumerator of diagnostic severities.
enum class Severity { Info, Warning, Error, Fatal };

/// @return true iff |a| is more than, or of equal severity to |b|.
inline bool operator>=(Severity a, Severity b) {
  return static_cast<int>(a) >= static_cast<int>(b);
}

/// Diagnostic holds all the information for a single compiler diagnostic
/// message.
class Diagnostic {
 public:
  /// severity is the severity of the diagnostic message.
  Severity severity = Severity::Error;
  /// source is the location of the diagnostic.
  Source source;
  /// message is the text associated with the diagnostic.
  std::string message;
};

/// List is a container of Diagnostic messages.
class List {
 public:
  /// iterator is the type used for range based iteration.
  using iterator = std::vector<Diagnostic>::const_iterator;

  /// Constructs the list with no elements.
  List();

  /// Copy constructor. Copies the diagnostics from |list| into this list.
  /// @param list the list of diagnostics to copy into this list.
  List(std::initializer_list<Diagnostic> list);

  /// Copy constructor. Copies the diagnostics from |list| into this list.
  /// @param list the list of diagnostics to copy into this list.
  List(const List& list);

  /// Move constructor. Moves the diagnostics from |list| into this list.
  /// @param list the list of diagnostics to move into this list.
  List(List&& list);
  ~List();

  /// Assignment operator. Copies the diagnostics from |list| into this list.
  /// @param list the list to copy into this list.
  /// @return this list.
  List& operator=(const List& list);

  /// Assignment move operator. Moves the diagnostics from |list| into this
  /// list.
  /// @param list the list to move into this list.
  /// @return this list.
  List& operator=(List&& list);

  /// adds a diagnostic to the end of this list.
  /// @param diag the diagnostic to append to this list.
  void add(Diagnostic&& diag) {
    entries_.emplace_back(std::move(diag));
    if (diag.severity >= Severity::Error) {
      contains_errors_ = true;
    }
  }

  /// @returns true iff the diagnostic list contains errors diagnostics (or of
  /// higher severity).
  bool contains_errors() const { return contains_errors_; }
  /// @returns the number of entries in the list.
  size_t count() const { return entries_.size(); }
  /// @returns the first diagnostic in the list.
  iterator begin() const { return entries_.begin(); }
  /// @returns the last diagnostic in the list.
  iterator end() const { return entries_.end(); }

 private:
  std::vector<Diagnostic> entries_;
  bool contains_errors_ = false;
};

}  // namespace diag
}  // namespace tint

#endif  // SRC_DIAGNOSTIC_DIAGNOSTIC_H_
