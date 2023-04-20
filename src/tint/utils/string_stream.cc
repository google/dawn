// Copyright 2023 The Tint Authors.
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

#include "src/tint/utils/string_stream.h"

namespace tint::utils {

StringStream::StringStream() {
    sstream_.flags(sstream_.flags() | std::ios_base::showpoint | std::ios_base::fixed);
    sstream_.imbue(std::locale::classic());
    sstream_.precision(9);
}

StringStream::~StringStream() = default;

utils::StringStream& operator<<(utils::StringStream& out, CodePoint code_point) {
    if (code_point < 0x7f) {
        // See https://en.cppreference.com/w/cpp/language/escape
        switch (code_point) {
            case '\a':
                return out << R"('\a')";
            case '\b':
                return out << R"('\b')";
            case '\f':
                return out << R"('\f')";
            case '\n':
                return out << R"('\n')";
            case '\r':
                return out << R"('\r')";
            case '\t':
                return out << R"('\t')";
            case '\v':
                return out << R"('\v')";
        }
        return out << "'" << static_cast<char>(code_point) << "'";
    }
    return out << "'U+" << std::hex << code_point.value << "'";
}

}  // namespace tint::utils
