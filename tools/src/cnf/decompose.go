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

// / Decomposed holds a decomposed expression
// / See Decompose()
type Decomposed struct {
	Ands   []Ands  // The decomposed Ands
	Ors    []Ors   // The decomposed Ors
	Unarys []Unary // The decomposed Unarys
}

// Decompose returns the unique Ands, Ors and Unarys that make up the expression.
// If e has two or more OR expressions AND'd together, then Decomposed.Ands will
// hold the deduplicated AND expressions, otherwise Decomposed.Ands will be
// empty.
// If e has two or more Unary expressions OR'd together, then Decomposed.ORs
// will hold the deduplicated OR expressions, otherwise Decomposed.ORs will be
// empty.
// Decomposed.Unarys will hold all the deduplicated Unary expressions.
func Decompose(e Expr) Decomposed {
	ors := container.NewMap[Key, Ors]()
	unarys := container.NewMap[Key, Unary]()
	for _, o := range e {
		for _, u := range o {
			unarys.Add(u.Key(), u)
		}
		if len(o) > 1 {
			ors.Add(o.Key(), o)
		}
	}
	d := Decomposed{}
	if len(e) > 1 {
		d.Ands = []Ands{e}
	}
	if len(ors) > 0 {
		d.Ors = ors.Values()
	}
	if len(unarys) > 0 {
		d.Unarys = unarys.Values()
	}
	return d
}
