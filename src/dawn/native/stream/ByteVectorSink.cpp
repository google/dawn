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

#include "dawn/native/stream/ByteVectorSink.h"

#include "dawn/native/stream/Stream.h"

namespace dawn::native::stream {

void* ByteVectorSink::GetSpace(size_t bytes) {
    size_t currentSize = this->size();
    this->resize(currentSize + bytes);
    return &this->operator[](currentSize);
}

template <>
void stream::Stream<ByteVectorSink>::Write(stream::Sink* sink, const ByteVectorSink& vec) {
    // For nested sinks, we do not record the length, and just copy the data so that it
    // appears flattened.
    size_t size = vec.size();
    if (size > 0) {
        void* ptr = sink->GetSpace(size);
        memcpy(ptr, vec.data(), size);
    }
}

std::ostream& operator<<(std::ostream& os, const ByteVectorSink& vec) {
    os << std::hex;
    for (const int b : vec) {
        os << std::setfill('0') << std::setw(2) << b << " ";
    }
    os << std::dec;
    return os;
}

}  // namespace dawn::native::stream
