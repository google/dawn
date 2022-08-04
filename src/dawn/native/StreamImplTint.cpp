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

#include "tint/tint.h"

namespace dawn::native {

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
                                                    const tint::sem::BindingPoint& p) {
    static_assert(offsetof(tint::sem::BindingPoint, group) == 0,
                  "Please update serialization for tint::sem::BindingPoint");
    static_assert(offsetof(tint::sem::BindingPoint, binding) == 4,
                  "Please update serialization for tint::sem::BindingPoint");
    static_assert(sizeof(tint::sem::BindingPoint) == 8,
                  "Please update serialization for tint::sem::BindingPoint");
    StreamIn(sink, p.group, p.binding);
}

// static
template <>
void stream::Stream<tint::transform::BindingPoints>::Write(
    stream::Sink* sink,
    const tint::transform::BindingPoints& points) {
    static_assert(offsetof(tint::transform::BindingPoints, plane_1) == 0,
                  "Please update serialization for tint::transform::BindingPoints");
    static_assert(offsetof(tint::transform::BindingPoints, params) == 8,
                  "Please update serialization for tint::transform::BindingPoints");
    static_assert(sizeof(tint::transform::BindingPoints) == 16,
                  "Please update serialization for tint::transform::BindingPoints");
    StreamIn(sink, points.plane_1, points.params);
}

template <>
void stream::Stream<tint::transform::VertexPulling::Config>::Write(
    stream::Sink* sink,
    const tint::transform::VertexPulling::Config& cfg) {
    StreamIn(sink, cfg.entry_point_name, cfg.vertex_state, cfg.pulling_group);
}

template <>
void stream::Stream<tint::transform::VertexBufferLayoutDescriptor>::Write(
    stream::Sink* sink,
    const tint::transform::VertexBufferLayoutDescriptor& layout) {
    using Layout = tint::transform::VertexBufferLayoutDescriptor;
    static_assert(offsetof(Layout, array_stride) == 0,
                  "Please update serialization for tint::transform::VertexBufferLayoutDescriptor");
    static_assert(offsetof(Layout, step_mode) == 4,
                  "Please update serialization for tint::transform::VertexBufferLayoutDescriptor");
    static_assert(offsetof(Layout, attributes) == 8,
                  "Please update serialization for tint::transform::VertexBufferLayoutDescriptor");
    StreamIn(sink, layout.array_stride, layout.step_mode, layout.attributes);
}

template <>
void stream::Stream<tint::transform::VertexAttributeDescriptor>::Write(
    stream::Sink* sink,
    const tint::transform::VertexAttributeDescriptor& attrib) {
    using Attrib = tint::transform::VertexAttributeDescriptor;
    static_assert(offsetof(Attrib, format) == 0,
                  "Please update serialization for tint::transform::VertexAttributeDescriptor");
    static_assert(offsetof(Attrib, offset) == 4,
                  "Please update serialization for tint::transform::VertexAttributeDescriptor");
    static_assert(offsetof(Attrib, shader_location) == 8,
                  "Please update serialization for tint::transform::VertexAttributeDescriptor");
    static_assert(sizeof(Attrib) == 12,
                  "Please update serialization for tint::transform::VertexAttributeDescriptor");
    StreamIn(sink, attrib.format, attrib.offset, attrib.shader_location);
}

}  // namespace dawn::native
