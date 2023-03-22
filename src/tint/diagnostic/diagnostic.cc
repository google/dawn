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

#include "src/tint/diagnostic/diagnostic.h"

#include <unordered_map>

#include "src/tint/diagnostic/formatter.h"

namespace tint::diag {

Diagnostic::Diagnostic() = default;
Diagnostic::Diagnostic(const Diagnostic&) = default;
Diagnostic::~Diagnostic() = default;
Diagnostic& Diagnostic::operator=(const Diagnostic&) = default;

List::List() = default;
List::List(std::initializer_list<Diagnostic> list) : entries_(list) {}
List::List(const List& rhs) = default;

List::List(List&& rhs) = default;

List::~List() = default;

List& List::operator=(const List& rhs) = default;

List& List::operator=(List&& rhs) = default;

std::string List::str() const {
    diag::Formatter::Style style;
    style.print_newline_at_end = false;
    return Formatter{style}.format(*this);
}

std::ostream& operator<<(std::ostream& out, const List& list) {
    out << list.str();
    return out;
}

}  // namespace tint::diag
