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

#ifndef SRC_TINT_SEM_BUILTIN_H_
#define SRC_TINT_SEM_BUILTIN_H_

#include <string>
#include <vector>

#include "src/tint/builtin/extension.h"
#include "src/tint/builtin/function.h"
#include "src/tint/sem/call_target.h"
#include "src/tint/sem/pipeline_stage_set.h"
#include "src/tint/utils/hash.h"

namespace tint::sem {

/// Determines if the given `i` is a coarse derivative
/// @param i the builtin type
/// @returns true if the given derivative is coarse.
bool IsCoarseDerivativeBuiltin(builtin::Function i);

/// Determines if the given `i` is a fine derivative
/// @param i the builtin type
/// @returns true if the given derivative is fine.
bool IsFineDerivativeBuiltin(builtin::Function i);

/// Determine if the given `i` is a derivative builtin
/// @param i the builtin type
/// @returns true if the given `i` is a derivative builtin
bool IsDerivativeBuiltin(builtin::Function i);

/// Determines if the given `i` is a texture operation builtin
/// @param i the builtin type
/// @returns true if the given `i` is a texture operation builtin
bool IsTextureBuiltin(builtin::Function i);

/// Determines if the given `i` is a image query builtin
/// @param i the builtin type
/// @returns true if the given `i` is a image query builtin
bool IsImageQueryBuiltin(builtin::Function i);

/// Determines if the given `i` is a data packing builtin
/// @param i the builtin
/// @returns true if the given `i` is a data packing builtin
bool IsDataPackingBuiltin(builtin::Function i);

/// Determines if the given `i` is a data unpacking builtin
/// @param i the builtin
/// @returns true if the given `i` is a data unpacking builtin
bool IsDataUnpackingBuiltin(builtin::Function i);

/// Determines if the given `i` is a barrier builtin
/// @param i the builtin
/// @returns true if the given `i` is a barrier builtin
bool IsBarrierBuiltin(builtin::Function i);

/// Determines if the given `i` is a atomic builtin
/// @param i the builtin
/// @returns true if the given `i` is a atomic builtin
bool IsAtomicBuiltin(builtin::Function i);

/// Determins if the given `i` is a DP4a builtin
/// @param i the builtin
/// @returns true if the given `i` is a DP4a builtin
bool IsDP4aBuiltin(builtin::Function i);

/// Builtin holds the semantic information for a builtin function.
class Builtin final : public utils::Castable<Builtin, CallTarget> {
  public:
    /// Constructor
    /// @param type the builtin type
    /// @param return_type the return type for the builtin call
    /// @param parameters the parameters for the builtin overload
    /// @param eval_stage the earliest evaluation stage for a call to the builtin
    /// @param supported_stages the pipeline stages that this builtin can be used in
    /// @param is_deprecated true if the particular overload is considered deprecated
    /// @param must_use true if the builtin was annotated with `@must_use`
    Builtin(builtin::Function type,
            const type::Type* return_type,
            utils::VectorRef<Parameter*> parameters,
            EvaluationStage eval_stage,
            PipelineStageSet supported_stages,
            bool is_deprecated,
            bool must_use);

    /// Destructor
    ~Builtin() override;

    /// @return the type of the builtin
    builtin::Function Type() const { return type_; }

    /// @return the pipeline stages that this builtin can be used in
    PipelineStageSet SupportedStages() const { return supported_stages_; }

    /// @return true if the builtin overload is considered deprecated
    bool IsDeprecated() const { return is_deprecated_; }

    /// @returns the name of the builtin function type. The spelling, including
    /// case, matches the name in the WGSL spec.
    const char* str() const;

    /// @returns true if builtin is a coarse derivative builtin
    bool IsCoarseDerivative() const;

    /// @returns true if builtin is a fine a derivative builtin
    bool IsFineDerivative() const;

    /// @returns true if builtin is a derivative builtin
    bool IsDerivative() const;

    /// @returns true if builtin is a texture operation builtin
    bool IsTexture() const;

    /// @returns true if builtin is a image query builtin
    bool IsImageQuery() const;

    /// @returns true if builtin is a data packing builtin
    bool IsDataPacking() const;

    /// @returns true if builtin is a data unpacking builtin
    bool IsDataUnpacking() const;

    /// @returns true if builtin is a barrier builtin
    bool IsBarrier() const;

    /// @returns true if builtin is a atomic builtin
    bool IsAtomic() const;

    /// @returns true if builtin is a DP4a builtin (defined in the extension
    /// chromium_experimental_DP4a)
    bool IsDP4a() const;

    /// @returns true if intrinsic may have side-effects (i.e. writes to at least
    /// one of its inputs)
    bool HasSideEffects() const;

    /// @returns the required extension of this builtin function. Returns
    /// builtin::Extension::kNone if no extension is required.
    builtin::Extension RequiredExtension() const;

  private:
    const builtin::Function type_;
    const PipelineStageSet supported_stages_;
    const bool is_deprecated_;
};

/// Constant value used by the degrees() builtin
static constexpr double kRadToDeg = 57.295779513082322865;

/// Constant value used by the radians() builtin
static constexpr double kDegToRad = 0.017453292519943295474;

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for tint::sem::Builtin
template <>
class hash<tint::sem::Builtin> {
  public:
    /// @param i the Builtin to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::Builtin& i) const {
        return tint::utils::Hash(i.Type(), i.SupportedStages(), i.ReturnType(), i.Parameters(),
                                 i.IsDeprecated());
    }
};

}  // namespace std

#endif  // SRC_TINT_SEM_BUILTIN_H_
