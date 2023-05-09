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

#include "src/tint/writer/text_generator.h"

#include <algorithm>
#include <limits>

#include "src/tint/utils/map.h"

namespace tint::writer {

TextGenerator::TextGenerator(const Program* program)
    : program_(program), builder_(ProgramBuilder::Wrap(program)) {}

TextGenerator::~TextGenerator() = default;

std::string TextGenerator::UniqueIdentifier(const std::string& prefix) {
    return builder_.Symbols().New(prefix).Name();
}

std::string TextGenerator::StructName(const type::Struct* s) {
    auto name = s->Name().Name();
    if (name.size() > 1 && name[0] == '_' && name[1] == '_') {
        name = utils::GetOrCreate(builtin_struct_names_, s,
                                  [&] { return UniqueIdentifier(name.substr(2)); });
    }
    return name;
}

TextGenerator::LineWriter::LineWriter(TextBuffer* buf) : buffer(buf) {}

TextGenerator::LineWriter::LineWriter(LineWriter&& other) {
    buffer = other.buffer;
    other.buffer = nullptr;
}

TextGenerator::LineWriter::~LineWriter() {
    if (buffer) {
        buffer->Append(os.str());
    }
}

TextGenerator::TextBuffer::TextBuffer() = default;
TextGenerator::TextBuffer::~TextBuffer() = default;

void TextGenerator::TextBuffer::IncrementIndent() {
    current_indent += 2;
}

void TextGenerator::TextBuffer::DecrementIndent() {
    current_indent = std::max(2u, current_indent) - 2u;
}

void TextGenerator::TextBuffer::Append(const std::string& line) {
    lines.emplace_back(Line{current_indent, line});
}

void TextGenerator::TextBuffer::Insert(const std::string& line, size_t before, uint32_t indent) {
    if (TINT_UNLIKELY(before >= lines.size())) {
        diag::List d;
        TINT_ICE(Writer, d) << "TextBuffer::Insert() called with before >= lines.size()\n"
                            << "  before:" << before << "\n"
                            << "  lines.size(): " << lines.size();
        return;
    }
    using DT = decltype(lines)::difference_type;
    lines.insert(lines.begin() + static_cast<DT>(before), Line{indent, line});
}

void TextGenerator::TextBuffer::Append(const TextBuffer& tb) {
    for (auto& line : tb.lines) {
        // TODO(bclayton): inefficient, consider optimizing
        lines.emplace_back(Line{current_indent + line.indent, line.content});
    }
}

void TextGenerator::TextBuffer::Insert(const TextBuffer& tb, size_t before, uint32_t indent) {
    if (TINT_UNLIKELY(before >= lines.size())) {
        diag::List d;
        TINT_ICE(Writer, d) << "TextBuffer::Insert() called with before >= lines.size()\n"
                            << "  before:" << before << "\n"
                            << "  lines.size(): " << lines.size();
        return;
    }
    size_t idx = 0;
    for (auto& line : tb.lines) {
        // TODO(bclayton): inefficient, consider optimizing
        using DT = decltype(lines)::difference_type;
        lines.insert(lines.begin() + static_cast<DT>(before + idx),
                     Line{indent + line.indent, line.content});
        idx++;
    }
}

std::string TextGenerator::TextBuffer::String(uint32_t indent /* = 0 */) const {
    utils::StringStream ss;
    for (auto& line : lines) {
        if (!line.content.empty()) {
            for (uint32_t i = 0; i < indent + line.indent; i++) {
                ss << " ";
            }
            ss << line.content;
        }
        ss << std::endl;
    }
    return ss.str();
}

TextGenerator::ScopedParen::ScopedParen(utils::StringStream& stream) : s(stream) {
    s << "(";
}

TextGenerator::ScopedParen::~ScopedParen() {
    s << ")";
}

TextGenerator::ScopedIndent::ScopedIndent(TextGenerator* generator)
    : ScopedIndent(generator->current_buffer_) {}

TextGenerator::ScopedIndent::ScopedIndent(TextBuffer* buffer) : buffer_(buffer) {
    buffer_->IncrementIndent();
}
TextGenerator::ScopedIndent::~ScopedIndent() {
    buffer_->DecrementIndent();
}

}  // namespace tint::writer
