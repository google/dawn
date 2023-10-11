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

import "dawn.googlesource.com/dawn/tools/src/container"

// Expr is a boolean expression, expressed in a Conjunctive Normal Form.
// Expr is an alias to Ands, which represent all the OR expressions that are
// AND'd together.
type Expr = Ands

// Ands is a slice of Ors
// Ands holds a sequence of OR'd expressions that are AND'd together:
//
//	Ors[0] && Ors[1] && ... && Ors[n-1]
type Ands []Ors

// Ors is a slice of Unary
// Ors holds a sequence of unary expressions that are OR'd together:
//
//	Unary[0] || Unary[1] || ... || Unary[n-1]
type Ors []Unary

// Unary is a negatable variable
type Unary struct {
	// If true, Var is negated
	Negate bool
	// The name of the variable
	Var string
}

// Remove returns a new expression with all the And expressions of o removed from e
func (e Expr) Remove(o Expr) Expr {
	set := container.NewSet[Key]()
	for _, expr := range o {
		set.Add(expr.Key())
	}
	out := Expr{}
	for _, expr := range e {
		if !set.Contains(expr.Key()) {
			out = append(out, expr)
		}
	}
	return out
}
