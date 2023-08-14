// Copyright 2023 The Dawn Authors
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

package transform_test

import (
	"fmt"
	"reflect"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/transform"
)

func check(got, expect any) error {
	if !reflect.DeepEqual(got, expect) {
		return fmt.Errorf(`
got:    %v
expect: %v
`, got, expect)
	}
	return nil
}

func TestFilter(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out := transform.Filter(in, func(i int) bool {
		return i != 8
	})
	if e := check(out, []int{5, 2, 4}); e != nil {
		t.Error(e)
	}
}

func TestFlatten(t *testing.T) {
	in := [][]int{{5, 8}, {2}, {4, 6, 5}}
	out := transform.Flatten(in)
	if e := check(out, []int{5, 8, 2, 4, 6, 5}); e != nil {
		t.Error(e)
	}
}

func TestSlice(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.Slice(in, func(i int) (int, error) {
		return i + 1, nil
	})
	if e := check(out, []int{6, 9, 3, 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestSliceErr(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.Slice(in, func(i int) (int, error) {
		return 0, fmt.Errorf("E(%v)", i)
	})
	if e := check(out, ([]int)(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), "E(5)"); e != nil {
		t.Error(e)
	}
}

func TestGoSlice(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.GoSlice(in, func(i int) (int, error) {
		return i + 1, nil
	})
	if e := check(out, []int{6, 9, 3, 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestGoSliceErr(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.GoSlice(in, func(i int) (int, error) {
		return 0, fmt.Errorf("E(%v)", i)
	})
	if e := check(out, ([]int)(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), `E(2)
E(4)
E(5)
E(8)`); e != nil {
		t.Error(e)
	}
}
