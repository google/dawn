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

#include "src/writer/float_to_string.h"

#include <limits>
#include <sstream>

namespace tint {
namespace writer {

std::string FloatToString(float f) {
  std::stringstream ss;
  ss.flags(ss.flags() | std::ios_base::showpoint | std::ios_base::fixed);
  ss.precision(std::numeric_limits<float>::max_digits10);
  ss << f;
  auto str = ss.str();
  while (str.length() >= 2 && str[str.size() - 1] == '0' &&
         str[str.size() - 2] != '.') {
    str.pop_back();
  }
  return str;
}

}  // namespace writer
}  // namespace tint
