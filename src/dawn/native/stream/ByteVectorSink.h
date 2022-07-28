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

#ifndef SRC_DAWN_NATIVE_STREAM_BYTEVECTORSINK_H_
#define SRC_DAWN_NATIVE_STREAM_BYTEVECTORSINK_H_

#include <ostream>
#include <vector>

#include "dawn/native/stream/Sink.h"

namespace dawn::native::stream {

// Implementation of stream::Sink backed by a byte vector.
class ByteVectorSink : public std::vector<uint8_t>, public Sink {
  public:
    using std::vector<uint8_t>::vector;

    // Implementation of stream::Sink
    void* GetSpace(size_t bytes) override;
};

// Stream operator for ByteVectorSink for debugging.
std::ostream& operator<<(std::ostream& os, const ByteVectorSink& key);

}  // namespace dawn::native::stream

#endif  // SRC_DAWN_NATIVE_STREAM_BYTEVECTORSINK_H_
