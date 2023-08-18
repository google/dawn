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

// String returns the expression in a C-like syntax
func (e Expr) String() string {
	return e.Format(" && ", " || ", "!")
}

// String returns the expression in a C-like syntax
func (o Ors) String() string {
	return o.Format(" || ", "!")
}

// String returns the expression in a C-like syntax
func (u Unary) String() string {
	return u.Format("!")
}

// Format returns the expression using the provided operators
func (e Expr) Format(and, or, not string) string {
	parts, _ := transform.Slice(e, func(ors Ors) (string, error) {
		if len(e) > 1 && len(ors) > 1 {
			return "(" + ors.Format(or, not) + ")", nil
		}
		return ors.Format(or, not), nil
	})
	return strings.Join(parts, and)
}

// Format returns the expression using the provided operators
func (o Ors) Format(or, not string) string {
	parts, _ := transform.Slice(o, func(u Unary) (string, error) {
		return u.Format(not), nil
	})
	return strings.Join(parts, or)
}

// Format returns the expression using the provided operator
func (u Unary) Format(not string) string {
	if u.Negate {
		return "(" + not + u.Var + ")"
	}
	return u.Var
}
