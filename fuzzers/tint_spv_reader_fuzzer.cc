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

#include <vector>

#include "src/reader/spirv/parser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  size_t sizeInU32 = size / sizeof(uint32_t);
  const uint32_t* u32Data = reinterpret_cast<const uint32_t*>(data);
  std::vector<uint32_t> input(u32Data, u32Data + sizeInU32);

  if (input.size() != 0) {
    tint::Context ctx;
    tint::reader::spirv::Parser parser(&ctx, input);
    parser.Parse();
  }

  return 0;
}
