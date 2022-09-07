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

#include "dawn/native/stream/Stream.h"

#include <string>

#include "dawn/native/Limits.h"

namespace dawn::native::stream {

template <>
void Stream<std::string>::Write(Sink* s, const std::string& t) {
    StreamIn(s, t.length());
    size_t size = t.length() * sizeof(char);
    if (size > 0) {
        void* ptr = s->GetSpace(size);
        memcpy(ptr, t.data(), size);
    }
}

template <>
MaybeError Stream<std::string>::Read(Source* s, std::string* t) {
    size_t length;
    DAWN_TRY(StreamOut(s, &length));
    const void* ptr;
    DAWN_TRY(s->Read(&ptr, length));
    *t = std::string(static_cast<const char*>(ptr), length);
    return {};
}

template <>
void Stream<std::string_view>::Write(Sink* s, const std::string_view& t) {
    StreamIn(s, t.length());
    size_t size = t.length() * sizeof(char);
    if (size > 0) {
        void* ptr = s->GetSpace(size);
        memcpy(ptr, t.data(), size);
    }
}

template <>
void Stream<std::wstring_view>::Write(Sink* s, const std::wstring_view& t) {
    StreamIn(s, t.length());
    size_t size = t.length() * sizeof(wchar_t);
    if (size > 0) {
        void* ptr = s->GetSpace(size);
        memcpy(ptr, t.data(), size);
    }
}

}  // namespace dawn::native::stream
