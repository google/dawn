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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either exprel or implied.
// See the License for the specific language governing permilions and
// limitations under the License.

package list_test

import (
	"reflect"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/list"
)

// A simple implementation of list.List. Many methods are just stubs
type customList struct{}

func (customList) Count() int                                { return 3 }
func (customList) Get(i int) interface{}                     { return 10 + i*10 }
func (customList) Set(i int, v interface{})                  {}
func (customList) Append(v interface{})                      {}
func (customList) Copy(dst, src, count int)                  {}
func (customList) CopyFrom(l list.List, dst, src, count int) {}
func (customList) Resize(count int)                          {}
func (customList) ElementType() reflect.Type                 { return nil }

var _ list.List = customList{} // Interface compliance check

func TestNew(t *testing.T) {
	l := list.New(reflect.TypeOf(0), 3)

	if n := l.Count(); n != 3 {
		t.Errorf("Count(0): %v", n)
	}
	if n := l.Get(0); n != 0 {
		t.Errorf("Get(0): %v", n)
	}
	if n := l.Get(1); n != 0 {
		t.Errorf("Get(1): %v", n)
	}
	if n := l.Get(2); n != 0 {
		t.Errorf("Get(2): %v", n)
	}
}

func TestCopy(t *testing.T) {
	slice := []int{1, 2, 3}
	l := list.Wrap(&slice)

	c := list.Copy(l)

	if n := c.Count(); n != 3 {
		t.Errorf("Count(0): %v", n)
	}
	if n := c.Get(0); n != 1 {
		t.Errorf("Get(0): %v", n)
	}
	if n := c.Get(1); n != 2 {
		t.Errorf("Get(1): %v", n)
	}
	if n := c.Get(2); n != 3 {
		t.Errorf("Get(2): %v", n)
	}
}

func TestListCount(t *testing.T) {
	slice := make([]int, 5)
	l := list.Wrap(&slice)

	if c := l.Count(); c != 5 {
		t.Errorf("Count() is %v", c)
	}
}

func TestListGrow(t *testing.T) {
	slice := []int{}
	l := list.Wrap(&slice)

	l.Resize(10)

	if len(slice) != 10 {
		t.Errorf("len(slice) after Resize(10) is %v", len(slice))
	}
}

func TestListShrink(t *testing.T) {
	slice := make([]int, 10)
	l := list.Wrap(&slice)

	l.Resize(5)

	if len(slice) != 5 {
		t.Errorf("len(slice) after Resize(5) is %v", len(slice))
	}
}

func TestListCopy(t *testing.T) {
	slice := []int{0, 10, 20, 0, 0, 0}
	l := list.Wrap(&slice)

	l.Copy(3, 1, 2)

	if !reflect.DeepEqual(slice, []int{0, 10, 20, 10, 20, 0}) {
		t.Errorf("after Copy(), slice: %v", slice)
	}
}

func TestListCopyFromList(t *testing.T) {
	sliceA := []int{10, 20, 30, 40, 50, 60}
	lA := list.Wrap(&sliceA)

	sliceB := []int{1, 2, 3, 4, 5, 6}
	lB := list.Wrap(&sliceB)

	lA.CopyFrom(lB, 1, 2, 3)

	if !reflect.DeepEqual(sliceA, []int{10, 3, 4, 5, 50, 60}) {
		t.Errorf("after CopyFrom(), slice: %v", sliceA)
	}
}

func TestListCopyFromCustomList(t *testing.T) {
	sliceA := []int{10, 20, 30, 40, 50, 60}
	lA := list.Wrap(&sliceA)

	lA.CopyFrom(customList{}, 1, 2, 3)

	if !reflect.DeepEqual(sliceA, []int{10, 30, 40, 50, 50, 60}) {
		t.Errorf("after CopyFrom(), slice: %v", sliceA)
	}
}

func TestListGet(t *testing.T) {
	slice := []int{0, 10, 20, 10, 20}
	l := list.Wrap(&slice)

	if n := l.Get(0); n != 0 {
		t.Errorf("Get(0): %v", n)
	}
	if n := l.Get(1); n != 10 {
		t.Errorf("Get(1): %v", n)
	}
	if n := l.Get(2); n != 20 {
		t.Errorf("Get(2): %v", n)
	}
	if n := l.Get(3); n != 10 {
		t.Errorf("Get(3): %v", n)
	}
	if n := l.Get(4); n != 20 {
		t.Errorf("Get(4): %v", n)
	}
}

func TestListSet(t *testing.T) {
	slice := []int{0, 10, 20, 10, 20}
	l := list.Wrap(&slice)

	l.Set(0, 50)
	l.Set(2, 90)
	l.Set(4, 60)

	if !reflect.DeepEqual(slice, []int{50, 10, 90, 10, 60}) {
		t.Errorf("after Set(), slice: %v", slice)
	}
}

func TestListAppendItem(t *testing.T) {
	slice := []int{1, 2, 3}
	l := list.Wrap(&slice)

	l.Append(9)

	if c := len(slice); c != 4 {
		t.Errorf("len(slice): %v", 4)
	}
	if n := slice[3]; n != 9 {
		t.Errorf("slice[3]: %v", n)
	}
}

func TestListAppendItems(t *testing.T) {
	slice := []int{1, 2, 3}
	l := list.Wrap(&slice)

	l.Append([]int{9, 8, 7})

	if !reflect.DeepEqual(slice, []int{1, 2, 3, 9, 8, 7}) {
		t.Errorf("after Append(), slice: %v", slice)
	}
}

func TestListAppendList(t *testing.T) {
	sliceA := []int{1, 2, 3}
	lA := list.Wrap(&sliceA)

	sliceB := []int{9, 8, 7}
	lB := list.Wrap(&sliceB)

	lA.Append(lB)

	if !reflect.DeepEqual(sliceA, []int{1, 2, 3, 9, 8, 7}) {
		t.Errorf("after Append(), sliceA: %v", sliceA)
	}
	if !reflect.DeepEqual(sliceB, []int{9, 8, 7}) {
		t.Errorf("after Append(), sliceB: %v", sliceB)
	}
}

func TestListAppendCustomList(t *testing.T) {
	sliceA := []int{1, 2, 3}
	lA := list.Wrap(&sliceA)

	lA.Append(customList{})

	if !reflect.DeepEqual(sliceA, []int{1, 2, 3, 10, 20, 30}) {
		t.Errorf("after Append(), sliceA: %v", sliceA)
	}
}
