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

#ifndef SRC_TRANSFORM_DECOMPOSE_STORAGE_ACCESS_H_
#define SRC_TRANSFORM_DECOMPOSE_STORAGE_ACCESS_H_

#include <string>

#include "src/ast/internal_decoration.h"
#include "src/transform/transform.h"

namespace tint {

// Forward declarations
class CloneContext;

namespace transform {

/// DecomposeStorageAccess is a transform used to replace storage buffer
/// accesses with a combination of load / store functions on primitive types.
class DecomposeStorageAccess : public Transform {
 public:
  /// Intrinsic is an InternalDecoration that's used to decorate a stub function
  /// so that the HLSL transforms this into calls to
  /// `[RW]ByteAddressBuffer.Load[N]()` or `[RW]ByteAddressBuffer.Store[N]()`,
  /// with a possible cast.
  class Intrinsic : public Castable<Intrinsic, ast::InternalDecoration> {
   public:
    /// Storage access intrinsic type
    enum Type {
      kLoadU32,       // `[RW]ByteAddressBuffer.Load()`
      kLoadF32,       // `asfloat([RW]ByteAddressBuffer.Load())`
      kLoadI32,       // `asint([RW]ByteAddressBuffer.Load())`
      kLoadVec2U32,   // `[RW]ByteAddressBuffer.Load2()`
      kLoadVec2F32,   // `asfloat([RW]ByteAddressBuffer.Load2())`
      kLoadVec2I32,   // `asint([RW]ByteAddressBuffer.Load2())`
      kLoadVec3U32,   // `[RW]ByteAddressBuffer.Load3()`
      kLoadVec3F32,   // `asfloat([RW]ByteAddressBuffer.Load3())`
      kLoadVec3I32,   // `asint([RW]ByteAddressBuffer.Load3())`
      kLoadVec4U32,   // `[RW]ByteAddressBuffer.Load4()`
      kLoadVec4F32,   // `asfloat([RW]ByteAddressBuffer.Load4())`
      kLoadVec4I32,   // `asint([RW]ByteAddressBuffer.Load4())`
      kStoreU32,      // `RWByteAddressBuffer.Store()`
      kStoreF32,      // `asfloat(RWByteAddressBuffer.Store())`
      kStoreI32,      // `asint(RWByteAddressBuffer.Store())`
      kStoreVec2U32,  // `RWByteAddressBuffer.Store2()`
      kStoreVec2F32,  // `asfloat(RWByteAddressBuffer.Store2())`
      kStoreVec2I32,  // `asint(RWByteAddressBuffer.Store2())`
      kStoreVec3U32,  // `RWByteAddressBuffer.Store3()`
      kStoreVec3F32,  // `asfloat(RWByteAddressBuffer.Store3())`
      kStoreVec3I32,  // `asint(RWByteAddressBuffer.Store3())`
      kStoreVec4U32,  // `RWByteAddressBuffer.Store4()`
      kStoreVec4F32,  // `asfloat(RWByteAddressBuffer.Store4())`
      kStoreVec4I32,  // `asint(RWByteAddressBuffer.Store4())`
    };

    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    /// @param ty the type of the intrinsic
    Intrinsic(ProgramID program_id, Type ty);
    /// Destructor
    ~Intrinsic() override;

    /// @return a short description of the internal decoration which will be
    /// displayed as `[[internal(<name>)]]`
    std::string Name() const override;

    /// Performs a deep clone of this object using the CloneContext `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned object
    Intrinsic* Clone(CloneContext* ctx) const override;

    /// The type of the intrinsic
    Type const type;
  };

  /// Constructor
  DecomposeStorageAccess();
  /// Destructor
  ~DecomposeStorageAccess() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

  struct State;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_DECOMPOSE_STORAGE_ACCESS_H_
