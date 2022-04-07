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

package lut_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/list"
	"dawn.googlesource.com/dawn/tools/src/lut"
)

func TestCompactWithFragments(t *testing.T) {
	runes := []rune{}
	lut := lut.New(list.Wrap(&runes))
	indices := []*int{
		lut.Add([]rune("the life in your")),
		lut.Add([]rune("in your life that count")),
		lut.Add([]rune("In the end,")),
		lut.Add([]rune("the life in")),
		lut.Add([]rune("count. It's the")),
		lut.Add([]rune("years")),
		lut.Add([]rune("in your years.")),
		lut.Add([]rune("it's not the years in")),
		lut.Add([]rune("not the years")),
		lut.Add([]rune("not the years in your")),
		lut.Add([]rune("end, it's")),
	}

	lut.Compact()

	expect := "In the end, it's not the years in your life that count. It's the life in your years."
	got := string(runes)
	if got != expect {
		t.Errorf("Compact result was not as expected\nExpected: '%v'\nGot:      '%v'", expect, got)
	}
	expectedIndices := []int{
		61, //                                                              the life in your
		31, //                                in your life that count
		0,  // In the end,
		61, //                                                              the life in
		49, //                                                  count. It's the
		25, //                          years
		70, //                                                                       in your years.
		12, //             it's not the years in
		17, //                  not the years
		17, //                  not the years in your
		7,  //        end, it's
	}
	for i, p := range indices {
		if expected, got := expectedIndices[i], *p; expected != got {
			t.Errorf("Index %v was not expected. Expected %v, got %v", i, expected, got)
		}
	}
}
