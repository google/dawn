// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_FUZZERS_TRANSFORM_BUILDER_H_
#define SRC_TINT_FUZZERS_TRANSFORM_BUILDER_H_

#include <string>
#include <vector>

#include "include/tint/tint.h"

#include "src/tint/fuzzers/data_builder.h"
#include "src/tint/fuzzers/shuffle_transform.h"
#include "src/tint/transform/binding_remapper.h"
#include "src/tint/transform/robustness.h"

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
    transform::Manager* manager() { return &manager_; }

    /// @returns data for transforms
    transform::DataMap* data_map() { return &data_map_; }

    /// Adds a transform and needed data to |manager_| and |data_map_|.
    /// @tparam T - A class that inherits from transform::Transform and has an
    ///             explicit specialization in AddTransformImpl.
    template <typename T>
    void AddTransform() {
        static_assert(std::is_base_of<transform::Transform, T>::value,
                      "T is not a transform::Transform");
        AddTransformImpl<T>::impl(this);
    }

    /// Helper that invokes Add*Transform for all of the platform independent
    /// passes.
    void AddPlatformIndependentPasses() {
        AddTransform<transform::FirstIndexOffset>();
        AddTransform<transform::BindingRemapper>();
        AddTransform<transform::Renamer>();
        AddTransform<transform::SingleEntryPoint>();
        AddTransform<transform::VertexPulling>();
    }

  private:
    DataBuilder builder_;
    transform::Manager manager_;
    transform::DataMap data_map_;

    DataBuilder* builder() { return &builder_; }

    /// Implementation of AddTransform, specialized for each transform that is
    /// implemented. Default implementation intentionally deleted to cause compile
    /// error if unimplemented type passed in.
    /// @tparam T - A fuzzer transform
    template <typename T>
    struct AddTransformImpl;

    /// Implementation of AddTransform for ShuffleTransform
    template <>
    struct AddTransformImpl<ShuffleTransform> {
        /// Add instance of ShuffleTransform to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) {
            tb->manager()->Add<ShuffleTransform>(tb->builder_.build<size_t>());
        }
    };

    /// Implementation of AddTransform for transform::Robustness
    template <>
    struct AddTransformImpl<transform::Robustness> {
        /// Add instance of transform::Robustness to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) { tb->manager()->Add<transform::Robustness>(); }
    };

    /// Implementation of AddTransform for transform::FirstIndexOffset
    template <>
    struct AddTransformImpl<transform::FirstIndexOffset> {
        /// Add instance of transform::FirstIndexOffset to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) {
            struct Config {
                uint32_t group;
                uint32_t binding;
            };

            Config config = tb->builder()->build<Config>();

            tb->data_map()->Add<tint::transform::FirstIndexOffset::BindingPoint>(config.binding,
                                                                                 config.group);
            tb->manager()->Add<transform::FirstIndexOffset>();
        }
    };

    /// Implementation of AddTransform for transform::BindingRemapper
    template <>
    struct AddTransformImpl<transform::BindingRemapper> {
        /// Add instance of transform::BindingRemapper to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) {
            struct Config {
                uint8_t old_group;
                uint8_t old_binding;
                uint8_t new_group;
                uint8_t new_binding;
                builtin::Access new_access;
            };

            std::vector<Config> configs = tb->builder()->vector<Config>();
            transform::BindingRemapper::BindingPoints binding_points;
            transform::BindingRemapper::AccessControls accesses;
            for (const auto& config : configs) {
                binding_points[{config.old_binding, config.old_group}] = {config.new_binding,
                                                                          config.new_group};
                accesses[{config.old_binding, config.old_group}] = config.new_access;
            }

            tb->data_map()->Add<transform::BindingRemapper::Remappings>(
                binding_points, accesses, tb->builder()->build<bool>());
            tb->manager()->Add<transform::BindingRemapper>();
        }
    };

    /// Implementation of AddTransform for transform::Renamer
    template <>
    struct AddTransformImpl<transform::Renamer> {
        /// Add instance of transform::Renamer to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) { tb->manager()->Add<transform::Renamer>(); }
    };

    /// Implementation of AddTransform for transform::SingleEntryPoint
    template <>
    struct AddTransformImpl<transform::SingleEntryPoint> {
        /// Add instance of transform::SingleEntryPoint to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) {
            auto input = tb->builder()->build<std::string>();
            transform::SingleEntryPoint::Config cfg(input);

            tb->data_map()->Add<transform::SingleEntryPoint::Config>(cfg);
            tb->manager()->Add<transform::SingleEntryPoint>();
        }
    };  // struct AddTransformImpl<transform::SingleEntryPoint>

    /// Implementation of AddTransform for transform::VertexPulling
    template <>
    struct AddTransformImpl<transform::VertexPulling> {
        /// Add instance of transform::VertexPulling to TransformBuilder
        /// @param tb - TransformBuilder to add transform to
        static void impl(TransformBuilder* tb) {
            transform::VertexPulling::Config cfg;
            cfg.vertex_state = tb->builder()->vector<transform::VertexBufferLayoutDescriptor>(
                GenerateVertexBufferLayoutDescriptor);
            cfg.pulling_group = tb->builder()->build<uint32_t>();

            tb->data_map()->Add<transform::VertexPulling::Config>(cfg);
            tb->manager()->Add<transform::VertexPulling>();
        }

      private:
        /// Generate an instance of transform::VertexAttributeDescriptor
        /// @param b - DataBuilder to use
        static transform::VertexAttributeDescriptor GenerateVertexAttributeDescriptor(
            DataBuilder* b) {
            transform::VertexAttributeDescriptor desc{};
            desc.format = b->enum_class<transform::VertexFormat>(
                static_cast<uint8_t>(transform::VertexFormat::kLastEntry) + 1);
            desc.offset = b->build<uint32_t>();
            desc.shader_location = b->build<uint32_t>();
            return desc;
        }

        /// Generate an instance of VertexBufferLayoutDescriptor
        /// @param b - DataBuilder to use
        static transform::VertexBufferLayoutDescriptor GenerateVertexBufferLayoutDescriptor(
            DataBuilder* b) {
            transform::VertexBufferLayoutDescriptor desc;
            desc.array_stride = b->build<uint32_t>();
            desc.step_mode = b->enum_class<transform::VertexStepMode>(
                static_cast<uint8_t>(transform::VertexStepMode::kLastEntry) + 1);
            desc.attributes =
                b->vector<transform::VertexAttributeDescriptor>(GenerateVertexAttributeDescriptor);
            return desc;
        }
    };
};  // class TransformBuilder

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_TRANSFORM_BUILDER_H_
