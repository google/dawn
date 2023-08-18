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

import "dawn.googlesource.com/dawn/tools/src/transform"

// And returns a new expression that represents (lhs ∧ rhs)
func And(lhs, rhs Expr) Expr {
	return append(append(Expr{}, lhs...), rhs...)
}

// Or returns a new expression that represents (lhs ∨ rhs)
func Or(lhs, rhs Expr) Expr {
	if len(lhs) == 0 {
		return rhs
	}
	if len(rhs) == 0 {
		return lhs
	}
	out := Expr{}
	for _, aOrs := range lhs {
		for _, bOrs := range rhs {
			out = append(out, append(append(Ors{}, aOrs...), bOrs...))
		}
	}
	return out
}

// Not returns a new expression that represents (¬expr)
func Not(expr Expr) Expr {
	// Transform each of the OR expressions into AND expressions where each unary is negated.
	out := Expr{}
	for _, o := range expr {
		ands, _ := transform.Slice(o, func(u Unary) (Ors, error) {
			return Ors{Unary{Negate: !u.Negate, Var: u.Var}}, nil
		})
		out = Or(out, ands)
	}
	return out
}
