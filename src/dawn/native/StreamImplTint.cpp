// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/stream/Stream.h"

#include "dawn/native/TintUtils.h"
#include "tint/tint.h"

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
    tint::wgsl::writer::Options options{};
    StreamIn(sink, tint::wgsl::writer::Generate(p, options)->wgsl);
#else
    // TODO(crbug.com/dawn/1481): We shouldn't need to write back to WGSL if we have a CacheKey
    // built from the initial shader module input. Then, we would never need to parse the program
    // and write back out to WGSL.
    DAWN_UNREACHABLE();
#endif
}

#if TINT_BUILD_MSL_WRITER
// static
template <>
void stream::Stream<tint::msl::writer::Options>::Write(stream::Sink* sink,
                                                       const tint::msl::writer::Options& options) {
    StreamInTintObject(options, sink);
}

#endif  // TINT_BUILD_MSL_WRITER

template <>
void stream::Stream<tint::PixelLocalOptions>::Write(stream::Sink* sink,
                                                    const tint::PixelLocalOptions& options) {
    StreamInTintObject(options, sink);
}

#if TINT_BUILD_GLSL_WRITER
// static
template <>
void stream::Stream<tint::glsl::writer::Options>::Write(
    stream::Sink* sink,
    const tint::glsl::writer::Options& options) {
    StreamInTintObject(options, sink);
}

// static
template <>
void stream::Stream<tint::glsl::writer::Version>::Write(
    stream::Sink* sink,
    const tint::glsl::writer::Version& version) {
    StreamInTintObject(version, sink);
}

// static
template <>
void stream::Stream<tint::glsl::writer::SamplerTexturePair>::Write(
    stream::Sink* sink,
    const tint::glsl::writer::SamplerTexturePair& options) {
    StreamInTintObject(options, sink);
}

#endif  // TINT_BUILD_GLSL_WRITER

// static
template <>
void stream::Stream<tint::BindingPoint>::Write(stream::Sink* sink,
                                               const tint::BindingPoint& point) {
    StreamInTintObject(point, sink);
}

// static
template <>
MaybeError stream::Stream<tint::BindingPoint>::Read(Source* s, tint::BindingPoint* point) {
    DAWN_TRY(StreamOut(s, &point->group));
    DAWN_TRY(StreamOut(s, &point->binding));
    return {};
}

#if TINT_BUILD_SPV_WRITER
// static
template <>
void stream::Stream<tint::spirv::writer::Options>::Write(
    stream::Sink* sink,
    const tint::spirv::writer::Options& options) {
    StreamInTintObject(options, sink);
}

// static
template <>
void stream::Stream<tint::spirv::writer::Bindings>::Write(
    stream::Sink* sink,
    const tint::spirv::writer::Bindings& bindings) {
    StreamInTintObject(bindings, sink);
}

// static
template <>
void stream::Stream<tint::spirv::writer::binding::ExternalTexture>::Write(
    stream::Sink* sink,
    const tint::spirv::writer::binding::ExternalTexture& et) {
    StreamInTintObject(et, sink);
}

// static
template <>
void stream::Stream<tint::spirv::writer::binding::BindingInfo>::Write(
    stream::Sink* sink,
    const tint::spirv::writer::binding::BindingInfo& point) {
    StreamInTintObject(point, sink);
}
#endif  // TINT_BUILD_SPV_WRITER

// static
template <>
void stream::Stream<tint::ExternalTextureOptions::BindingPoints>::Write(
    stream::Sink* sink,
    const tint::ExternalTextureOptions::BindingPoints& point) {
    StreamInTintObject(point, sink);
}

// static
template <>
void stream::Stream<tint::ExternalTextureOptions>::Write(
    stream::Sink* sink,
    const tint::ExternalTextureOptions& points) {
    StreamInTintObject(points, sink);
}

// static
template <>
void stream::Stream<tint::ast::transform::VertexPulling::Config>::Write(
    stream::Sink* sink,
    const tint::ast::transform::VertexPulling::Config& cfg) {
    StreamInTintObject(cfg, sink);
}

// static
template <>
void stream::Stream<tint::ast::transform::SubstituteOverride::Config>::Write(
    stream::Sink* sink,
    const tint::ast::transform::SubstituteOverride::Config& cfg) {
    StreamInTintObject(cfg, sink);
}

// static
template <>
void stream::Stream<tint::OverrideId>::Write(stream::Sink* sink, const tint::OverrideId& id) {
    StreamInTintObject(id, sink);
}

// static
template <>
void stream::Stream<tint::ast::transform::VertexBufferLayoutDescriptor>::Write(
    stream::Sink* sink,
    const tint::ast::transform::VertexBufferLayoutDescriptor& layout) {
    StreamInTintObject(layout, sink);
}

// static
template <>
void stream::Stream<tint::ast::transform::VertexAttributeDescriptor>::Write(
    stream::Sink* sink,
    const tint::ast::transform::VertexAttributeDescriptor& attrib) {
    StreamInTintObject(attrib, sink);
}

// static
template <>
void stream::Stream<tint::ArrayLengthFromUniformOptions>::Write(
    stream::Sink* sink,
    const tint::ArrayLengthFromUniformOptions& options) {
    StreamInTintObject(options, sink);
}

// static
template <>
void stream::Stream<tint::TextureBuiltinsFromUniformOptions>::Write(
    stream::Sink* sink,
    const tint::TextureBuiltinsFromUniformOptions& options) {
    StreamInTintObject(options, sink);
}

// static
template <>
void stream::Stream<tint::BindingRemapperOptions>::Write(
    stream::Sink* sink,
    const tint::BindingRemapperOptions& options) {
    StreamInTintObject(options, sink);
}

}  // namespace dawn::native
