// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_DIAGNOSTIC_DIAGNOSTIC_H_
#define SRC_TINT_UTILS_DIAGNOSTIC_DIAGNOSTIC_H_

#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/traits/traits.h"

namespace tint::diag {

/// Severity is an enumerator of diagnostic severities.
enum class Severity { Note, Warning, Error, InternalCompilerError, Fatal };

/// @return true iff `a` is more than, or of equal severity to `b`
inline bool operator>=(Severity a, Severity b) {
    return static_cast<int>(a) >= static_cast<int>(b);
}

/// System is an enumerator of Tint systems that can be the originator of a diagnostic message.
enum class System {
    AST,
    Builtin,
    Clone,
    Constant,
    Inspector,
    Intrinsics,
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
    Unknown,
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
    /// A shared pointer to a Source::File. Only used if the diagnostic Source
    /// points to a file that was created specifically for this diagnostic
    /// (usually an ICE).
    std::shared_ptr<Source::File> owned_file = nullptr;
};

/// List is a container of Diagnostic messages.
class List {
  public:
    /// The iterator type for this List
    using iterator = VectorIterator<const Diagnostic>;

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
        entries_.Push(std::move(diag));
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
    size_t count() const { return entries_.Length(); }
    /// @returns true if the diagnostics list is empty
    bool empty() const { return entries_.IsEmpty(); }
    /// @returns the number of entrise in the diagnostics list
    size_t size() const { return entries_.Length(); }
    /// @returns the first diagnostic in the list.
    iterator begin() const { return entries_.begin(); }
    /// @returns the last diagnostic in the list.
    iterator end() const { return entries_.end(); }

    /// @returns a formatted string of all the diagnostics in this list.
    std::string str() const;

  private:
    Vector<Diagnostic, 0> entries_;
    size_t error_count_ = 0;
};

/// Write the diagnostic list to the given stream
/// @param out the output stream
/// @param list the list to emit
/// @returns the output stream
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, const List& list) {
    return out << list.str();
}

}  // namespace tint::diag

#endif  // SRC_TINT_UTILS_DIAGNOSTIC_DIAGNOSTIC_H_
