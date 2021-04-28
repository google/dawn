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

#ifndef SRC_WRITER_SPIRV_GENERATOR_H_
#define SRC_WRITER_SPIRV_GENERATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "src/writer/spirv/binary_writer.h"
#include "src/writer/writer.h"

namespace tint {
namespace writer {
namespace spirv {

/// Class to generate SPIR-V from a Tint program
class Generator : public writer::Writer {
 public:
  /// Constructor
  /// @param program the program to convert
  explicit Generator(const Program* program);

  /// Destructor
  ~Generator() override;

  /// Generates the result data
  /// @returns true on successful generation; false otherwise
  bool Generate() override;

  /// @returns the result data
  const std::vector<uint32_t>& result() const { return writer_->result(); }

 private:
  std::unique_ptr<Builder> builder_;
  std::unique_ptr<BinaryWriter> writer_;
};

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_GENERATOR_H_
