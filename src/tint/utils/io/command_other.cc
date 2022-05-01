// Copyright 2021 The Tint Authors.
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

#include "src/tint/utils/io/command.h"

namespace tint::utils {

Command::Command(const std::string&) {}

Command Command::LookPath(const std::string&) {
    return Command("");
}

bool Command::Found() const {
    return false;
}

Command::Output Command::Exec(std::initializer_list<std::string>) const {
    Output out;
    out.err = "Command not supported by this target";
    return out;
}

}  // namespace tint::utils
