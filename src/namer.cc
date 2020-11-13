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

#include "src/namer.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace tint {

Namer::Namer() = default;

Namer::~Namer() = default;

bool Namer::IsMapped(const std::string& name) {
  auto it = name_map_.find(name);
  return it != name_map_.end();
}

HashingNamer::HashingNamer() = default;

HashingNamer::~HashingNamer() = default;

std::string HashingNamer::NameFor(const std::string& name) {
  auto it = name_map_.find(name);
  if (it != name_map_.end()) {
    return it->second;
  }

  std::stringstream ret_name;
  ret_name << "tint_";

  ret_name << std::hex << std::setfill('0') << std::setw(2);
  for (size_t i = 0; i < name.size(); ++i) {
    ret_name << static_cast<uint32_t>(name[i]);
  }

  name_map_[name] = ret_name.str();
  return ret_name.str();
}

NoopNamer::NoopNamer() = default;

NoopNamer::~NoopNamer() = default;

std::string NoopNamer::NameFor(const std::string& name) {
  name_map_[name] = name;
  return name;
}

}  // namespace tint
