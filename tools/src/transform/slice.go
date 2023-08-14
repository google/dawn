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

package transform

import (
	"runtime"
	"sync"
)

// Filter returns a new slice of T with all the items that match the given predicate
func Filter[T any](items []T, pred func(T) bool) []T {
	out := make([]T, 0, len(items))
	for _, item := range items {
		if pred(item) {
			out = append(out, item)
		}
	}
	return out
}

// Flatten takes a slice of slices, and returns a linearized slice
func Flatten[T any, S ~[]T](lists []S) S {
	flat := S{}
	for _, list := range lists {
		flat = append(flat, list...)
	}
	return flat
}

// Slice returns a new slice by transforming each element with the function fn
func Slice[IN any, OUT any](in []IN, fn func(in IN) (OUT, error)) ([]OUT, error) {
	out := make([]OUT, len(in))
	for i, el := range in {
		o, err := fn(el)
		if err != nil {
			return nil, err
		}
		out[i] = o
	}

	return out, nil
}

// GoSlice returns a new slice by transforming each element with the function
// fn, called by multiple go-routines.
func GoSlice[IN any, OUT any](in []IN, fn func(in IN) (OUT, error)) ([]OUT, error) {
	// Create a channel of indices
	indices := make(chan int, 256)
	go func() {
		for i := range in {
			indices <- i
		}
		close(indices)
	}()

	out := make([]OUT, len(in))
	errs := make(Errors, len(in))

	// Kick a number of workers to process the elements
	numWorkers := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numWorkers)
	for worker := 0; worker < numWorkers; worker++ {
		go func() {
			defer wg.Done()
			for idx := range indices {
				out[idx], errs[idx] = fn(in[idx])
			}
		}()
	}
	wg.Wait()

	errs = Filter(errs, func(e error) bool { return e != nil })
	if len(errs) > 0 {
		return nil, errs
	}

	return out, nil
}
