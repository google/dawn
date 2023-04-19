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

#ifndef SRC_TINT_SEM_NODE_H_
#define SRC_TINT_SEM_NODE_H_

#include "src/tint/utils/castable.h"

namespace tint::sem {

/// Node is the base class for all semantic nodes
class Node : public utils::Castable<Node> {
  public:
    /// Constructor
    Node();

    /// Copy constructor
    Node(const Node&);

    /// Destructor
    ~Node() override;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_NODE_H_
