// Copyright 2022 The Dawn Authors
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

#ifndef COMMON_VERISON_AUTOGEN_H_
#define COMMON_VERISON_AUTOGEN_H_

#include <string_view>

namespace dawn {

// The version string should either be a valid git hash or empty.
static constexpr std::string_view kDawnVersion("{{get_version()}}");
static_assert(kDawnVersion.size() == 40 || kDawnVersion.size() == 0);

} // namespace dawn

#endif  // COMMON_VERISON_AUTOGEN_H_
