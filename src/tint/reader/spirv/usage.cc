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

#include "src/tint/reader/spirv/usage.h"

#include "src/tint/utils/string_stream.h"

namespace tint::reader::spirv {

Usage::Usage() {}
Usage::Usage(const Usage& other) = default;
Usage::~Usage() = default;

utils::StringStream& Usage::operator<<(utils::StringStream& out) const {
    out << "Usage(";
    if (IsSampler()) {
        out << "Sampler(";
        if (is_comparison_sampler_) {
            out << " comparison";
        }
        out << " )";
    }
    if (IsTexture()) {
        out << "Texture(";
        if (is_sampled_) {
            out << " is_sampled";
        }
        if (is_multisampled_) {
            out << " ms";
        }
        if (is_depth_) {
            out << " depth";
        }
        if (is_storage_read_) {
            out << " read";
        }
        if (is_storage_write_) {
            out << " write";
        }
        out << " )";
    }
    out << ")";
    return out;
}

bool Usage::IsValid() const {
    // Check sampler state internal consistency.
    if (is_comparison_sampler_ && !is_sampler_) {
        return false;
    }

    // Check texture state.
    // |is_texture_| is implied by any of the later texture-based properties.
    if ((IsStorageTexture() || is_sampled_ || is_multisampled_ || is_depth_) && !is_texture_) {
        return false;
    }
    if (is_texture_) {
        // Multisampled implies sampled.
        if (is_multisampled_) {
            if (!is_sampled_) {
                return false;
            }
        }
        // Depth implies sampled.
        if (is_depth_) {
            if (!is_sampled_) {
                return false;
            }
        }

        // Sampled can't be storage.
        if (is_sampled_) {
            if (IsStorageTexture()) {
                return false;
            }
        }

        // Storage can't be sampled.
        if (IsStorageTexture()) {
            if (is_sampled_) {
                return false;
            }
        }
        // Storage texture can't also be a sampler.
        if (IsStorageTexture()) {
            if (is_sampler_) {
                return false;
            }
        }

        // Can't be both read and write.  This is a restriction in WebGPU.
        if (is_storage_read_ && is_storage_write_) {
            return false;
        }
    }
    return true;
}

bool Usage::IsComplete() const {
    if (!IsValid()) {
        return false;
    }
    if (IsSampler()) {
        return true;
    }
    if (IsTexture()) {
        return is_sampled_ || IsStorageTexture();
    }
    return false;
}

bool Usage::operator==(const Usage& other) const {
    return is_sampler_ == other.is_sampler_ &&
           is_comparison_sampler_ == other.is_comparison_sampler_ &&
           is_texture_ == other.is_texture_ && is_sampled_ == other.is_sampled_ &&
           is_multisampled_ == other.is_multisampled_ && is_depth_ == other.is_depth_ &&
           is_storage_read_ == other.is_storage_read_ &&
           is_storage_write_ == other.is_storage_write_;
}

void Usage::Add(const Usage& other) {
    is_sampler_ = is_sampler_ || other.is_sampler_;
    is_comparison_sampler_ = is_comparison_sampler_ || other.is_comparison_sampler_;
    is_texture_ = is_texture_ || other.is_texture_;
    is_sampled_ = is_sampled_ || other.is_sampled_;
    is_multisampled_ = is_multisampled_ || other.is_multisampled_;
    is_depth_ = is_depth_ || other.is_depth_;
    is_storage_read_ = is_storage_read_ || other.is_storage_read_;
    is_storage_write_ = is_storage_write_ || other.is_storage_write_;
}

void Usage::AddSampler() {
    is_sampler_ = true;
}

void Usage::AddComparisonSampler() {
    AddSampler();
    is_comparison_sampler_ = true;
}

void Usage::AddTexture() {
    is_texture_ = true;
}

void Usage::AddStorageReadTexture() {
    AddTexture();
    is_storage_read_ = true;
}

void Usage::AddStorageWriteTexture() {
    AddTexture();
    is_storage_write_ = true;
}

void Usage::AddSampledTexture() {
    AddTexture();
    is_sampled_ = true;
}

void Usage::AddMultisampledTexture() {
    AddSampledTexture();
    is_multisampled_ = true;
}

void Usage::AddDepthTexture() {
    AddSampledTexture();
    is_depth_ = true;
}

std::string Usage::to_str() const {
    utils::StringStream ss;
    ss << *this;
    return ss.str();
}

}  // namespace tint::reader::spirv
