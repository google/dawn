// Copyright 2023 The Dawn Authors
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

#include <stdint.h>

namespace DawnLPMFuzzer {


static constexpr int kInstanceObjectId = 1;
static constexpr uint32_t kInvalidObjectId = {{ cmd_records["lpm_info"]["invalid object id"] }};

{% for type in by_category["object"] %}
    {% if type.name.get() in cmd_records["lpm_info"]["limits"] %}
        static constexpr int k{{ type.name.CamelCase() }}Limit = {{ cmd_records["lpm_info"]["limits"][type.name.get()] }};
    {% else %}
        static constexpr int k{{ type.name.CamelCase() }}Limit = {{ cmd_records["lpm_info"]["limits"]["default"] }};
    {% endif %}
{% endfor %}

} // namespace DawnLPMFuzzer
