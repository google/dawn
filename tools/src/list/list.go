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

// Package list provides utilities for handling lists of dynamically-typed elements
package list

import (
	"fmt"
	"reflect"
)

// List is an interface to a list of dynamically-typed elements
type List interface {
	// Count returns the number if items in the list
	Count() int

	// Get returns the element at the index i
	Get(i int) interface{}

	// Set assigns the element at the index i with v
	Set(i int, v interface{})

	// Append adds a single item, list, or slice of items to this List
	Append(v interface{})

	// Copy copies the elements at [dst..dst+count) to [src..src+count)
	Copy(dst, src, count int)

	// CopyFrom copies the elements [src..src+count) from the list l to the
	// elements [dst..dst+count) in this list
	CopyFrom(l List, dst, src, count int)

	// Reduces the size of the list to count elements
	Resize(count int)

	// ElementType returns the type of the elements of the list
	ElementType() reflect.Type
}

// Wrap returns a List that wraps a slice pointer
func Wrap(s interface{}) List {
	ptr := reflect.ValueOf(s)
	if ptr.Kind() != reflect.Ptr || ptr.Elem().Kind() != reflect.Slice {
		panic(fmt.Errorf("Wrap() must be called with a pointer to slice. Got: %T", s))
	}
	return list{ptr.Elem()}
}

// New returns a new list of element type elem for n items
func New(elem reflect.Type, count int) List {
	slice := reflect.SliceOf(elem)
	return list{reflect.MakeSlice(slice, count, count)}
}

// Copy makes a shallow copy of the list
func Copy(l List) List {
	out := New(l.ElementType(), l.Count())
	out.CopyFrom(l, 0, 0, l.Count())
	return out
}

type list struct{ v reflect.Value }

func (l list) Count() int {
	return l.v.Len()
}

func (l list) Get(i int) interface{} {
	return l.v.Index(i).Interface()
}

func (l list) Set(i int, v interface{}) {
	l.v.Index(i).Set(reflect.ValueOf(v))
}

func (l list) Append(v interface{}) {
	switch v := v.(type) {
	case list:
		l.v.Set(reflect.AppendSlice(l.v, reflect.Value(v.v)))
	case List:
		// v implements `List`, but isn't a `list`. Need to do a piece-wise copy
		items := make([]reflect.Value, v.Count())
		for i := range items {
			items[i] = reflect.ValueOf(v.Get(i))
		}
		l.v.Set(reflect.Append(l.v, items...))
	default:
		r := reflect.ValueOf(v)
		if r.Type() == l.v.Type() {
			l.v.Set(reflect.AppendSlice(l.v, r))
			return
		}
		l.v.Set(reflect.Append(l.v, reflect.ValueOf(v)))
	}
}

func (l list) Copy(dst, src, count int) {
	reflect.Copy(
		l.v.Slice(dst, dst+count),
		l.v.Slice(src, src+count),
	)
}

func (l list) CopyFrom(o List, dst, src, count int) {
	if o, ok := o.(list); ok {
		reflect.Copy(
			l.v.Slice(dst, dst+count),
			o.v.Slice(src, src+count),
		)
	}
	// v implements `List`, but isn't a `list`. Need to do a piece-wise copy
	items := make([]reflect.Value, count)
	for i := range items {
		l.Set(dst+i, o.Get(src+i))
	}
}

func (l list) Resize(count int) {
	new := reflect.MakeSlice(l.v.Type(), count, count)
	reflect.Copy(new, l.v)
	l.v.Set(new)
}

func (l list) ElementType() reflect.Type {
	return l.v.Type().Elem()
}
