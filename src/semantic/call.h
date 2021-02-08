// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_SEMANTIC_CALL_H_
#define SRC_SEMANTIC_CALL_H_

#include "src/semantic/expression.h"
#include "src/semantic/intrinsic.h"

namespace tint {
namespace semantic {

/// Call is the base class for semantic nodes that hold semantic information for
/// ast::CallExpression nodes.
class Call : public Castable<Call, Expression> {
 public:
  /// Constructor
  /// @param return_type the return type of the call
  explicit Call(type::Type* return_type);

  /// Destructor
  ~Call() override;
};

/// IntrinsicCall holds semantic information for ast::CallExpression nodes that
/// call intrinsic functions.
class IntrinsicCall : public Castable<IntrinsicCall, Call> {
 public:
  /// Constructor
  /// @param return_type the return type of the call
  /// @param intrinsic the call target intrinsic
  IntrinsicCall(type::Type* return_type, IntrinsicType intrinsic);

  /// Destructor
  ~IntrinsicCall() override;

  /// @returns the target intrinsic for the call
  IntrinsicType intrinsic() const { return intrinsic_; }

 private:
  IntrinsicType const intrinsic_;
};

/// TextureIntrinsicCall holds semantic information for ast::CallExpression
/// nodes that call intrinsic texture functions.
class TextureIntrinsicCall
    : public Castable<TextureIntrinsicCall, IntrinsicCall> {
 public:
  /// Parameters describes the parameters for the texture function.
  struct Parameters {
    /// kNotUsed is the constant that indicates the given parameter is not part
    /// of the texture function signature.
    static constexpr const size_t kNotUsed = ~static_cast<size_t>(0u);
    /// Index holds each of the possible parameter indices. If a parameter index
    /// is equal to `kNotUsed` then this parameter is not used by the function.
    struct Index {
      /// Constructor
      Index();
      /// Copy constructor
      Index(const Index&);
      /// `array_index` parameter index.
      size_t array_index = kNotUsed;
      /// `bias` parameter index.
      size_t bias = kNotUsed;
      /// `coords` parameter index.
      size_t coords = kNotUsed;
      /// `depth_ref` parameter index.
      size_t depth_ref = kNotUsed;
      /// `ddx` parameter index.
      size_t ddx = kNotUsed;
      /// `ddy` parameter index.
      size_t ddy = kNotUsed;
      /// `level` parameter index.
      size_t level = kNotUsed;
      /// `offset` parameter index.
      size_t offset = kNotUsed;
      /// `sampler` parameter index.
      size_t sampler = kNotUsed;
      /// `sample_index` parameter index.
      size_t sample_index = kNotUsed;
      /// `texture` parameter index.
      size_t texture = kNotUsed;
      /// `value` parameter index.
      size_t value = kNotUsed;
    };
    /// The indices of all possible parameters.
    Index idx;
    /// Total number of parameters.
    size_t count = 0;
  };

  /// Constructor
  /// @param return_type the return type of the call
  /// @param intrinsic the call target intrinsic
  /// @param params the overload parameter info
  TextureIntrinsicCall(type::Type* return_type,
                       IntrinsicType intrinsic,
                       const Parameters& params);

  /// Destructor
  ~TextureIntrinsicCall() override;

  /// @return the texture call's parameters
  const Parameters& Params() const { return params_; }

 private:
  const Parameters params_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_CALL_H_
