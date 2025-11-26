// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_ANALYSIS_SUBGROUP_MATRIX_H_
#define SRC_TINT_LANG_CORE_IR_ANALYSIS_SUBGROUP_MATRIX_H_

#include <cstdint>
#include <unordered_set>

#include "src/tint/utils/math/hash.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir::analysis {

enum class SubgroupMatrixType : uint8_t {
    kF16 = 0,
    kF32,
    kU8,
    kI8,
    kU32,
    kI32,
};

struct SubgroupMatrixMultiply {
    uint32_t M;
    uint32_t N;
    uint32_t K;

    SubgroupMatrixType input_type;
    SubgroupMatrixType output_type;

    bool operator==(const SubgroupMatrixMultiply& o) const {
        return M == o.M && N == o.N && K == o.K && input_type == o.input_type &&
               output_type == o.output_type;
    }
};

enum class SubgroupMatrixDirection : uint8_t {
    kInput,
    kResult,
};

struct SubgroupMatrixConfig {
    uint32_t columns;
    uint32_t rows;
    SubgroupMatrixType type;
    SubgroupMatrixDirection direction;

    bool operator==(const SubgroupMatrixConfig& o) const {
        return columns == o.columns && rows == o.rows && type == o.type && direction == o.direction;
    }
};

}  // namespace tint::core::ir::analysis

template <>
class std::hash<tint::core::ir::analysis::SubgroupMatrixMultiply> {
  public:
    inline std::size_t operator()(
        const tint::core::ir::analysis::SubgroupMatrixMultiply& sm) const {
        return tint::Hash(sm.M, sm.N, sm.K, sm.input_type, sm.output_type);
    }
};

template <>
class std::hash<tint::core::ir::analysis::SubgroupMatrixConfig> {
  public:
    inline std::size_t operator()(const tint::core::ir::analysis::SubgroupMatrixConfig sm) const {
        return tint::Hash(sm.columns, sm.rows, sm.type, sm.direction);
    }
};

namespace tint::core::ir::analysis {

struct SubgroupMatrixInfo {
    std::unordered_set<SubgroupMatrixMultiply> multiplies;
    std::unordered_set<SubgroupMatrixConfig> configs;
};

/// Gathers information about the subgroup matrix configurations used in the module.
///
/// This returns two fundamental types of information on the subgroup matrix uses.
///
/// 1. The Matrix multiply configurations
///   * This provides the `M`, `N`, `K`, input and output types for the
///     `subgroupMatrixMultiply` and `subgroupMatrtixMultiplyAccumulate` calls in the
///     module.  (The configs are de-duplicated so each combination is only returned once.)
///
/// 2. The used matrix configurations
///   * This provides the `C`, `R`, type and if it's left/right (input) or result usage for
///     every subgroup matrix seen in the file.  (The configs are de-duplicated so each
///     combination is only returned once.)
///
/// @param module the module to transform
/// @returns the subgroup matrix information
SubgroupMatrixInfo GatherSubgroupMatrixInfo(core::ir::Module& module);

}  // namespace tint::core::ir::analysis

#endif  // SRC_TINT_LANG_CORE_IR_ANALYSIS_SUBGROUP_MATRIX_H_
