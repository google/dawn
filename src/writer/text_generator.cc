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

#include "src/writer/text_generator.h"

#include <algorithm>
#include <limits>

namespace tint {
namespace writer {

TextGenerator::TextGenerator(const Program* program)
    : program_(program), builder_(ProgramBuilder::Wrap(program)) {}

TextGenerator::~TextGenerator() = default;

std::string TextGenerator::UniqueIdentifier(const std::string& prefix) {
  return builder_.Symbols().NameFor(builder_.Symbols().New(prefix));
}

std::string TextGenerator::TrimSuffix(std::string str,
                                      const std::string& suffix) {
  if (str.size() >= suffix.size()) {
    if (str.substr(str.size() - suffix.size(), suffix.size()) == suffix) {
      return str.substr(0, str.size() - suffix.size());
    }
  }
  return str;
}

TextGenerator::LineWriter::LineWriter(TextGenerator* generator)
    : gen(generator) {}

TextGenerator::LineWriter::LineWriter(LineWriter&& other) {
  gen = other.gen;
  other.gen = nullptr;
}

TextGenerator::LineWriter::~LineWriter() {
  if (gen) {
    gen->current_buffer_->Append(os.str());
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

void TextGenerator::TextBuffer::Append(const TextBuffer& tb) {
  for (auto& line : tb.lines) {
    lines.emplace_back(Line{current_indent + line.indent, line.content});
  }
}

std::string TextGenerator::TextBuffer::String() const {
  std::stringstream ss;
  for (auto& line : lines) {
    if (!line.content.empty()) {
      for (uint32_t i = 0; i < line.indent; i++) {
        ss << " ";
      }
      ss << line.content;
    }
    ss << std::endl;
  }
  return ss.str();
}

TextGenerator::ScopedParen::ScopedParen(std::ostream& stream) : s(stream) {
  s << "(";
}
TextGenerator::ScopedParen::~ScopedParen() {
  s << ")";
}

TextGenerator::ScopedIndent::ScopedIndent(TextGenerator* generator)
    : gen(generator) {
  gen->increment_indent();
}
TextGenerator::ScopedIndent::~ScopedIndent() {
  gen->decrement_indent();
}

}  // namespace writer
}  // namespace tint
