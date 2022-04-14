// Copyright 2022 The Tint Authors.
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

package utils_test

import (
	"dawn.googlesource.com/dawn/tools/src/utils"
	"testing"
)

func TestHash(t *testing.T) {
	type Test struct {
		a, b        []any
		expectEqual bool
	}
	for _, test := range []Test{
		{a: []any{1}, b: []any{1}, expectEqual: true},
		{a: []any{1}, b: []any{2}, expectEqual: false},
		{a: []any{1}, b: []any{1.0}, expectEqual: false},
		{a: []any{1.0}, b: []any{1.0}, expectEqual: true},
		{a: []any{'x'}, b: []any{'x'}, expectEqual: true},
		{a: []any{'x'}, b: []any{'y'}, expectEqual: false},
		{a: []any{1, 2}, b: []any{1, 2}, expectEqual: true},
		{a: []any{1, 2}, b: []any{1}, expectEqual: false},
		{a: []any{1, 2}, b: []any{1, 3}, expectEqual: false},
	} {
		hashA := utils.Hash(test.a...)
		hashB := utils.Hash(test.b...)
		equal := hashA == hashB
		if equal != test.expectEqual {
			t.Errorf("Hash(%v): %v\nHash(%v): %v", test.a, hashA, test.b, hashB)
		}
	}
}
