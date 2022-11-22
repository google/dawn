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

#include "dawn/native/TintUtils.h"
#include "tint/writer/array_length_from_uniform_options.h"

namespace dawn::native {

namespace {

template <typename OBJECT>
void StreamInTintObject(const OBJECT& object, stream::Sink* sink) {
    tint::ForeachField(object, [&](auto& field) { StreamIn(sink, field); });
}

}  // namespace

// static
template <>
void stream::Stream<tint::Program>::Write(stream::Sink* sink, const tint::Program& p) {
#if TINT_BUILD_WGSL_WRITER
    tint::writer::wgsl::Options options{};
    StreamIn(sink, tint::writer::wgsl::Generate(&p, options).wgsl);
#else
    // TODO(crbug.com/dawn/1481): We shouldn't need to write back to WGSL if we have a CacheKey
    // built from the initial shader module input. Then, we would never need to parse the program
    // and write back out to WGSL.
    UNREACHABLE();
#endif
}

// static
template <>
void stream::Stream<tint::sem::BindingPoint>::Write(stream::Sink* sink,
                                                    const tint::sem::BindingPoint& point) {
    StreamInTintObject(point, sink);
}

// static
template <>
void stream::Stream<tint::transform::MultiplanarExternalTexture::BindingPoints>::Write(
    stream::Sink* sink,
    const tint::transform::MultiplanarExternalTexture::BindingPoints& points) {
    StreamInTintObject(points, sink);
}

// static
template <>
void stream::Stream<tint::transform::VertexPulling::Config>::Write(
    stream::Sink* sink,
    const tint::transform::VertexPulling::Config& cfg) {
    StreamInTintObject(cfg, sink);
}

// static
template <>
void stream::Stream<tint::transform::SubstituteOverride::Config>::Write(
    stream::Sink* sink,
    const tint::transform::SubstituteOverride::Config& cfg) {
    StreamInTintObject(cfg, sink);
}

// static
template <>
void stream::Stream<tint::OverrideId>::Write(stream::Sink* sink, const tint::OverrideId& id) {
    StreamInTintObject(id, sink);
}

// static
template <>
void stream::Stream<tint::transform::VertexBufferLayoutDescriptor>::Write(
    stream::Sink* sink,
    const tint::transform::VertexBufferLayoutDescriptor& layout) {
    StreamInTintObject(layout, sink);
}

// static
template <>
void stream::Stream<tint::transform::VertexAttributeDescriptor>::Write(
    stream::Sink* sink,
    const tint::transform::VertexAttributeDescriptor& attrib) {
    StreamInTintObject(attrib, sink);
}

// static
template <>
void stream::Stream<tint::writer::ArrayLengthFromUniformOptions>::Write(
    stream::Sink* sink,
    const tint::writer::ArrayLengthFromUniformOptions& options) {
    StreamInTintObject(options, sink);
}

}  // namespace dawn::native
