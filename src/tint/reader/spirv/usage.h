// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_READER_SPIRV_USAGE_H_
#define SRC_TINT_READER_SPIRV_USAGE_H_

#include <string>

#include "src/tint/utils/string_stream.h"

namespace tint::reader::spirv {

/// Records the properties of a sampler or texture based on how it's used
/// by image instructions inside function bodies.
///
/// For example:
///
///   If %X is the "Image" parameter of an OpImageWrite instruction then
///    - The memory object declaration underlying %X will gain
///      AddStorageWriteTexture usage
///
///   If %Y is the "Sampled Image" parameter of an OpImageSampleDrefExplicitLod
///   instruction, and %Y is composed from sampler %YSam and image %YIm, then:
///    - The memory object declaration underlying %YSam will gain
///      AddComparisonSampler usage
///    - The memory object declaration unederlying %YIm will gain
///      AddSampledTexture and AddDepthTexture usages
class Usage {
  public:
    /// Constructor
    Usage();
    /// Copy constructor
    /// @param other the Usage to clone
    Usage(const Usage& other);
    /// Destructor
    ~Usage();

    /// @returns true if this usage is internally consistent
    bool IsValid() const;
    /// @returns true if the usage fully determines a WebGPU binding type.
    bool IsComplete() const;

    /// @returns true if this usage is a sampler usage.
    bool IsSampler() const { return is_sampler_; }
    /// @returns true if this usage is a comparison sampler usage.
    bool IsComparisonSampler() const { return is_comparison_sampler_; }

    /// @returns true if this usage is a texture usage.
    bool IsTexture() const { return is_texture_; }
    /// @returns true if this usage is a sampled texture usage.
    bool IsSampledTexture() const { return is_sampled_; }
    /// @returns true if this usage is a multisampled texture usage.
    bool IsMultisampledTexture() const { return is_multisampled_; }
    /// @returns true if this usage is a dpeth texture usage.
    bool IsDepthTexture() const { return is_depth_; }
    /// @returns true if this usage is a read-only storage texture
    bool IsStorageReadTexture() const { return is_storage_read_; }
    /// @returns true if this usage is a write-only storage texture
    bool IsStorageWriteTexture() const { return is_storage_write_; }

    /// @returns true if this is a storage texture.
    bool IsStorageTexture() const { return is_storage_read_ || is_storage_write_; }

    /// Emits this usage to the given stream
    /// @param out the output stream.
    /// @returns the modified stream.
    utils::StringStream& operator<<(utils::StringStream& out) const;

    /// Equality operator
    /// @param other the RHS of the equality test.
    /// @returns true if `other` is identical to `*this`
    bool operator==(const Usage& other) const;

    /// Adds the usages from another usage object.
    /// @param other the other usage
    void Add(const Usage& other);

    /// Records usage as a sampler.
    void AddSampler();
    /// Records usage as a comparison sampler.
    void AddComparisonSampler();

    /// Records usage as a texture of some kind.
    void AddTexture();
    /// Records usage as a read-only storage texture.
    void AddStorageReadTexture();
    /// Records usage as a write-only storage texture.
    void AddStorageWriteTexture();
    /// Records usage as a sampled texture.
    void AddSampledTexture();
    /// Records usage as a multisampled texture.
    void AddMultisampledTexture();
    /// Records usage as a depth texture.
    void AddDepthTexture();

    /// @returns this usage object as a string.
    std::string to_str() const;

  private:
    // Sampler properties.
    bool is_sampler_ = false;
    // A comparison sampler is always a sampler:
    //    |is_comparison_sampler_| implies |is_sampler_|
    bool is_comparison_sampler_ = false;

    // Texture properties.
    // |is_texture_| is always implied by any of the others below.
    bool is_texture_ = false;
    bool is_sampled_ = false;
    bool is_multisampled_ = false;  // This implies it's sampled as well.
    bool is_depth_ = false;
    bool is_storage_read_ = false;
    bool is_storage_write_ = false;
};

/// Writes the Usage to the stream
/// @param out the stream
/// @param u the Usage
/// @returns the stream so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& out, const Usage& u) {
    return u.operator<<(out);
}

}  // namespace tint::reader::spirv

#endif  // SRC_TINT_READER_SPIRV_USAGE_H_
