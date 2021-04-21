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

#ifndef SRC_SEM_TEXTURE_TYPE_H_
#define SRC_SEM_TEXTURE_TYPE_H_

#include "src/ast/texture.h"
#include "src/sem/type.h"

namespace tint {
namespace sem {

/// A texture type.
class Texture : public Castable<Texture, Type> {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  explicit Texture(ast::TextureDimension dim);
  /// Move constructor
  Texture(Texture&&);
  ~Texture() override;

  /// @returns the texture dimension
  ast::TextureDimension dim() const { return dim_; }

 private:
  ast::TextureDimension const dim_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_TEXTURE_TYPE_H_
