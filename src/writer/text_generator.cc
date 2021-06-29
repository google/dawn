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

#include <limits>

namespace tint {
namespace writer {

TextGenerator::TextGenerator(const Program* program)
    : program_(program), builder_(ProgramBuilder::Wrap(program)) {}

TextGenerator::~TextGenerator() = default;

void TextGenerator::make_indent() {
  make_indent(out_);
}

void TextGenerator::make_indent(std::ostream& out) const {
  for (size_t i = 0; i < indent_; i++) {
    out << " ";
  }
}

std::string TextGenerator::UniqueIdentifier(const std::string& prefix) {
  return builder_.Symbols().NameFor(builder_.Symbols().New(prefix));
}

TextGenerator::LineWriter::LineWriter(TextGenerator* generator)
    : gen(generator) {}

TextGenerator::LineWriter::LineWriter(LineWriter&& other) {
  gen = other.gen;
  other.gen = nullptr;
}

TextGenerator::LineWriter::~LineWriter() {
  if (gen) {
    auto str = os.str();
    if (!str.empty()) {
      gen->make_indent();
      gen->out_ << str;
    }
    gen->out_ << std::endl;
  }
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
