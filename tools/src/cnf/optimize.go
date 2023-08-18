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
	"dawn.googlesource.com/dawn/tools/src/container"
)

// Optimize returns the expression with all duplicate unary expressions in the
// ORs removed. The returned expression is sorted.
func Optimize(e Expr) Expr {
	m := container.NewMap[Key, Ors]()
	for _, o := range e {
		b := unique(o)
		m.Add(b.Key(), b)
	}
	return m.Values()
}

// unique returns the o with all duplicate unary expressions removed
// The returned OR expression list is sorted.
func unique(o Ors) Ors {
	m := container.NewMap[Key, Unary]()
	for _, u := range o {
		m.Add(u.Key(), u)
	}
	return m.Values()
}
