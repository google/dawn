// Copyright 2023 The Tint Authors.
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

package cnf

import (
	"strings"

	"dawn.googlesource.com/dawn/tools/src/transform"
)

// Key is a map key type returned by Key() methods on Ands, Ors and Unary.
type Key string

// Key returns a Key that can be used to sort the Ands
func (e Ands) Key() Key {
	parts, _ := transform.Slice(e, func(o Ors) (string, error) {
		return string(o.Key()), nil
	})
	return Key(strings.Join(parts, "&&"))
}

// Key returns a Key that can be used to sort the Ors
func (o Ors) Key() Key {
	parts, _ := transform.Slice(o, func(u Unary) (string, error) {
		return string(u.Key()), nil
	})
	return Key(strings.Join(parts, "||"))
}

// Key returns a Key that can be used to sort the Unary
func (u Unary) Key() Key {
	if u.Negate {
		return Key(u.Var + "!")
	}
	return Key(u.Var)
}
