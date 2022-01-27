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

#include "src/diagnostic/diagnostic.h"

#include <unordered_map>

#include "src/diagnostic/formatter.h"

namespace tint {
namespace diag {

List::List() = default;
List::List(std::initializer_list<Diagnostic> list) : entries_(list) {}
List::List(const List& rhs) {
  *this = rhs;
}

List::List(List&& rhs) = default;

List::~List() = default;

List& List::operator=(const List& rhs) {
  // Create copies of any of owned files, maintaining a map of rhs-file to
  // new-file.
  std::unordered_map<const Source::File*, const Source::File*> files;
  owned_files_.reserve(rhs.owned_files_.size());
  files.reserve(rhs.owned_files_.size());
  for (auto& rhs_file : rhs.owned_files_) {
    auto file = std::make_unique<Source::File>(*rhs_file);
    files.emplace(rhs_file.get(), file.get());
    owned_files_.emplace_back(std::move(file));
  }

  // Copy the diagnostic entries, then fix up pointers to the file copies.
  entries_ = rhs.entries_;
  for (auto& entry : entries_) {
    if (auto it = files.find(entry.source.file); it != files.end()) {
      entry.source.file = it->second;
    }
  }

  error_count_ = rhs.error_count_;
  return *this;
}

List& List::operator=(List&& rhs) = default;

std::string List::str() const {
  diag::Formatter::Style style;
  style.print_newline_at_end = false;
  return Formatter{style}.format(*this);
}

}  // namespace diag
}  // namespace tint
