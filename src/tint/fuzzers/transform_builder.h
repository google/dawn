// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_FUZZERS_TRANSFORM_BUILDER_H_
#define SRC_TINT_FUZZERS_TRANSFORM_BUILDER_H_

#include <string>
#include <vector>

#include "include/tint/tint.h"

#include "src/tint/fuzzers/data_builder.h"
#include "src/tint/fuzzers/shuffle_transform.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/robustness.h"

namespace tint::fuzzers {

/// Fuzzer utility class to build inputs for transforms and setup the transform
/// manager.
class TransformBuilder {
  public:
    /// @brief Initializes the internal builder using a seed value
    /// @param seed - seed value passed to engine
    explicit TransformBuilder(uint64_t seed) : builder_(seed) {}

    /// @brief Initializes the internal builder using seed data
    /// @param data - data fuzzer to calculate seed from
    /// @param size - size of data buffer
    explicit TransformBuilder(const uint8_t* data, size_t size) : builder_(data, size) {
        assert(data != nullptr && "|data| must be !nullptr");
    }

    ~TransformBuilder() = default;

    /// @returns manager for transforms
    ast::transform::Manager* manager() { return &manager_; }

    /// @returns data for transforms
    ast::transform::DataMap* data_map() { return &data_map_; }

    /// Adds a transform and needed data to |manager_| and |data_map_|.
    /// @tparam T - A class that inherits from ast::transform::Transform and has an
    ///             explicit specialization in AddTransformImpl.
    template <typename T>
    void AddTransform() {
        static_assert(std::is_base_of<ast::transform::Transform, T>::value,
                      "T is not a ast::transform::Transform");
        AddTransformImpl<T>::impl(this);
    }

    /// Helper that invokes Add*Transform for all of the platform independent
    /// passes.
    void AddPlatformIndependentPasses() {
        AddTransform<ast::transform::FirstIndexOffset>();
        AddTransform<ast::transform::BindingRemapper>();
        AddTransform<ast::transform::Renamer>();
        AddTransform<ast::transform::SingleEntryPoint>();
        AddTransform<ast::transform::VertexPulling>();
    }

  private:
    DataBuilder builder_;
    ast::transform::Manager manager_;
    ast::transform::DataMap data_map_;

    DataBuilder* builder() { return &builder_; }

    /// Implementation of AddTransform, specialized for each transform that is
    /// implemented. Default implementation intentionally deleted to cause compile
    /// error if unimplemented type passed in.
    /// @tparam T - A fuzzer transform
    template <typename T>
    struct AddTransformImpl;
};

/// Implementation of AddTransform for ShuffleTransform
template <>
struct TransformBuilder::AddTransformImpl<ShuffleTransform> {
    /// Add instance of ShuffleTransform to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) {
        tb->manager()->Add<ShuffleTransform>(tb->builder_.build<size_t>());
    }
};

/// Implementation of AddTransform for ast::transform::Robustness
template <>
struct TransformBuilder::AddTransformImpl<ast::transform::Robustness> {
    /// Add instance of ast::transform::Robustness to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) { tb->manager()->Add<ast::transform::Robustness>(); }
};

/// Implementation of AddTransform for ast::transform::FirstIndexOffset
template <>
struct TransformBuilder::AddTransformImpl<ast::transform::FirstIndexOffset> {
    /// Add instance of ast::transform::FirstIndexOffset to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) {
        struct Config {
            uint32_t group;
            uint32_t binding;
        };

        Config config = tb->builder()->build<Config>();

        tb->data_map()->Add<tint::ast::transform::FirstIndexOffset::BindingPoint>(config.binding,
                                                                                  config.group);
        tb->manager()->Add<ast::transform::FirstIndexOffset>();
    }
};

/// Implementation of AddTransform for ast::transform::BindingRemapper
template <>
struct TransformBuilder::AddTransformImpl<ast::transform::BindingRemapper> {
    /// Add instance of ast::transform::BindingRemapper to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) {
        struct Config {
            uint8_t old_group;
            uint8_t old_binding;
            uint8_t new_group;
            uint8_t new_binding;
            core::Access new_access;
        };

        std::vector<Config> configs = tb->builder()->vector<Config>();
        ast::transform::BindingRemapper::BindingPoints binding_points;
        ast::transform::BindingRemapper::AccessControls accesses;
        for (const auto& config : configs) {
            binding_points[{config.old_binding, config.old_group}] = {config.new_binding,
                                                                      config.new_group};
            accesses[{config.old_binding, config.old_group}] = config.new_access;
        }

        tb->data_map()->Add<ast::transform::BindingRemapper::Remappings>(
            binding_points, accesses, tb->builder()->build<bool>());
        tb->manager()->Add<ast::transform::BindingRemapper>();
    }
};

/// Implementation of AddTransform for ast::transform::Renamer
template <>
struct TransformBuilder::AddTransformImpl<ast::transform::Renamer> {
    /// Add instance of ast::transform::Renamer to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) { tb->manager()->Add<ast::transform::Renamer>(); }
};

/// Implementation of AddTransform for ast::transform::SingleEntryPoint
template <>
struct TransformBuilder::AddTransformImpl<ast::transform::SingleEntryPoint> {
    /// Add instance of ast::transform::SingleEntryPoint to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) {
        auto input = tb->builder()->build<std::string>();
        ast::transform::SingleEntryPoint::Config cfg(input);

        tb->data_map()->Add<ast::transform::SingleEntryPoint::Config>(cfg);
        tb->manager()->Add<ast::transform::SingleEntryPoint>();
    }
};

/// Implementation of AddTransform for ast::transform::VertexPulling
template <>
struct TransformBuilder::AddTransformImpl<ast::transform::VertexPulling> {
    /// Add instance of ast::transform::VertexPulling to TransformBuilder
    /// @param tb - TransformBuilder to add transform to
    static void impl(TransformBuilder* tb) {
        ast::transform::VertexPulling::Config cfg;
        cfg.vertex_state = tb->builder()->vector<ast::transform::VertexBufferLayoutDescriptor>(
            GenerateVertexBufferLayoutDescriptor);
        cfg.pulling_group = tb->builder()->build<uint32_t>();

        tb->data_map()->Add<ast::transform::VertexPulling::Config>(cfg);
        tb->manager()->Add<ast::transform::VertexPulling>();
    }

  private:
    /// Generate an instance of ast::transform::VertexAttributeDescriptor
    /// @param b - DataBuilder to use
    static ast::transform::VertexAttributeDescriptor GenerateVertexAttributeDescriptor(
        DataBuilder* b) {
        ast::transform::VertexAttributeDescriptor desc{};
        desc.format = b->enum_class<ast::transform::VertexFormat>(
            static_cast<uint8_t>(ast::transform::VertexFormat::kLastEntry) + 1);
        desc.offset = b->build<uint32_t>();
        desc.shader_location = b->build<uint32_t>();
        return desc;
    }

    /// Generate an instance of VertexBufferLayoutDescriptor
    /// @param b - DataBuilder to use
    static ast::transform::VertexBufferLayoutDescriptor GenerateVertexBufferLayoutDescriptor(
        DataBuilder* b) {
        ast::transform::VertexBufferLayoutDescriptor desc;
        desc.array_stride = b->build<uint32_t>();
        desc.step_mode = b->enum_class<ast::transform::VertexStepMode>(
            static_cast<uint8_t>(ast::transform::VertexStepMode::kLastEntry) + 1);
        desc.attributes =
            b->vector<ast::transform::VertexAttributeDescriptor>(GenerateVertexAttributeDescriptor);
        return desc;
    }
};

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_TRANSFORM_BUILDER_H_
