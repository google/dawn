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

#ifndef SRC_TINT_DIAGNOSTIC_DIAGNOSTIC_H_
#define SRC_TINT_DIAGNOSTIC_DIAGNOSTIC_H_

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "src/tint/source.h"

namespace tint::diag {

/// Severity is an enumerator of diagnostic severities.
enum class Severity { Note, Warning, Error, InternalCompilerError, Fatal };

/// @return true iff `a` is more than, or of equal severity to `b`
inline bool operator>=(Severity a, Severity b) {
    return static_cast<int>(a) >= static_cast<int>(b);
}

/// System is an enumerator of Tint systems that can be the originator of a
/// diagnostic message.
enum class System {
    AST,
    Clone,
    Constant,
    Inspector,
    IR,
    Program,
    ProgramBuilder,
    Reader,
    Resolver,
    Semantic,
    Symbol,
    Test,
    Transform,
    Type,
    Utils,
    Writer,
};

/// Diagnostic holds all the information for a single compiler diagnostic
/// message.
class Diagnostic {
  public:
    /// Constructor
    Diagnostic();
    /// Copy constructor
    Diagnostic(const Diagnostic&);
    /// Destructor
    ~Diagnostic();

    /// Copy assignment operator
    /// @return this diagnostic
    Diagnostic& operator=(const Diagnostic&);

    /// severity is the severity of the diagnostic message.
    Severity severity = Severity::Error;
    /// source is the location of the diagnostic.
    Source source;
    /// message is the text associated with the diagnostic.
    std::string message;
    /// system is the Tint system that raised the diagnostic.
    System system;
    /// code is the error code, for example a validation error might have the code
    /// `"v-0001"`.
    const char* code = nullptr;
    /// A shared pointer to a Source::File. Only used if the diagnostic Source
    /// points to a file that was created specifically for this diagnostic
    /// (usually an ICE).
    std::shared_ptr<Source::File> owned_file = nullptr;
};

/// List is a container of Diagnostic messages.
class List {
  public:
    /// iterator is the type used for range based iteration.
    using iterator = std::vector<Diagnostic>::const_iterator;

    /// Constructs the list with no elements.
    List();

    /// Copy constructor. Copies the diagnostics from `list` into this list.
    /// @param list the list of diagnostics to copy into this list.
    List(std::initializer_list<Diagnostic> list);

    /// Copy constructor. Copies the diagnostics from `list` into this list.
    /// @param list the list of diagnostics to copy into this list.
    List(const List& list);

    /// Move constructor. Moves the diagnostics from `list` into this list.
    /// @param list the list of diagnostics to move into this list.
    List(List&& list);

    /// Destructor
    ~List();

    /// Assignment operator. Copies the diagnostics from `list` into this list.
    /// @param list the list to copy into this list.
    /// @return this list.
    List& operator=(const List& list);

    /// Assignment move operator. Moves the diagnostics from `list` into this
    /// list.
    /// @param list the list to move into this list.
    /// @return this list.
    List& operator=(List&& list);

    /// adds a diagnostic to the end of this list.
    /// @param diag the diagnostic to append to this list.
    void add(Diagnostic&& diag) {
        if (diag.severity >= Severity::Error) {
            error_count_++;
        }
        entries_.emplace_back(std::move(diag));
    }

    /// adds a list of diagnostics to the end of this list.
    /// @param list the diagnostic to append to this list.
    void add(const List& list) {
        for (auto diag : list) {
            add(std::move(diag));
        }
    }

    /// adds the note message with the given Source to the end of this list.
    /// @param system the system raising the note message
    /// @param note_msg the note message
    /// @param source the source of the note diagnostic
    void add_note(System system, std::string_view note_msg, const Source& source) {
        diag::Diagnostic note{};
        note.severity = diag::Severity::Note;
        note.system = system;
        note.source = source;
        note.message = note_msg;
        add(std::move(note));
    }

    /// adds the warning message with the given Source to the end of this list.
    /// @param system the system raising the warning message
    /// @param warning_msg the warning message
    /// @param source the source of the warning diagnostic
    void add_warning(System system, std::string_view warning_msg, const Source& source) {
        diag::Diagnostic warning{};
        warning.severity = diag::Severity::Warning;
        warning.system = system;
        warning.source = source;
        warning.message = warning_msg;
        add(std::move(warning));
    }

    /// adds the error message without a source to the end of this list.
    /// @param system the system raising the error message
    /// @param err_msg the error message
    void add_error(System system, std::string_view err_msg) {
        diag::Diagnostic error{};
        error.severity = diag::Severity::Error;
        error.system = system;
        error.message = err_msg;
        add(std::move(error));
    }

    /// adds the error message with the given Source to the end of this list.
    /// @param system the system raising the error message
    /// @param err_msg the error message
    /// @param source the source of the error diagnostic
    void add_error(System system, std::string_view err_msg, const Source& source) {
        diag::Diagnostic error{};
        error.severity = diag::Severity::Error;
        error.system = system;
        error.source = source;
        error.message = err_msg;
        add(std::move(error));
    }

    /// adds the error message with the given code and Source to the end of this
    /// list.
    /// @param system the system raising the error message
    /// @param code the error code
    /// @param err_msg the error message
    /// @param source the source of the error diagnostic
    void add_error(System system,
                   const char* code,
                   std::string_view err_msg,
                   const Source& source) {
        diag::Diagnostic error{};
        error.code = code;
        error.severity = diag::Severity::Error;
        error.system = system;
        error.source = source;
        error.message = err_msg;
        add(std::move(error));
    }

    /// adds an internal compiler error message to the end of this list.
    /// @param system the system raising the error message
    /// @param err_msg the error message
    /// @param source the source of the internal compiler error
    /// @param file the Source::File owned by this diagnostic
    void add_ice(System system,
                 std::string_view err_msg,
                 const Source& source,
                 std::shared_ptr<Source::File> file) {
        diag::Diagnostic ice{};
        ice.severity = diag::Severity::InternalCompilerError;
        ice.system = system;
        ice.source = source;
        ice.message = err_msg;
        ice.owned_file = std::move(file);
        add(std::move(ice));
    }

    /// @returns true iff the diagnostic list contains errors diagnostics (or of
    /// higher severity).
    bool contains_errors() const { return error_count_ > 0; }
    /// @returns the number of error diagnostics (or of higher severity).
    size_t error_count() const { return error_count_; }
    /// @returns the number of entries in the list.
    size_t count() const { return entries_.size(); }
    /// @returns true if the diagnostics list is empty
    bool empty() const { return entries_.empty(); }
    /// @returns the number of entrise in the diagnostics list
    size_t size() const { return entries_.size(); }
    /// @returns the first diagnostic in the list.
    iterator begin() const { return entries_.begin(); }
    /// @returns the last diagnostic in the list.
    iterator end() const { return entries_.end(); }

    /// @returns a formatted string of all the diagnostics in this list.
    std::string str() const;

  private:
    std::vector<Diagnostic> entries_;
    size_t error_count_ = 0;
};

/// Write the diagnostic list to the given stream
/// @param out the output stream
/// @param list the list to emit
/// @returns the output stream
std::ostream& operator<<(std::ostream& out, const List& list);

}  // namespace tint::diag

#endif  // SRC_TINT_DIAGNOSTIC_DIAGNOSTIC_H_
