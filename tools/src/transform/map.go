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

// Map returns a new map by transforming each entry with the function fn
func Map[IN_KEY comparable, IN_VAL any, OUT_KEY comparable, OUT_VAL any](in map[IN_KEY]IN_VAL, fn func(k IN_KEY, v IN_VAL) (OUT_KEY, OUT_VAL, error)) (map[OUT_KEY]OUT_VAL, error) {
	out := make(map[OUT_KEY]OUT_VAL, len(in))
	errs := Errors{}
	for inKey, inValue := range in {
		outKey, outVal, err := fn(inKey, inValue)
		if err != nil {
			errs = append(errs, err)
		}
		out[outKey] = outVal
	}

	if len(errs) > 0 {
		return nil, errs
	}

	return out, nil
}

type keyVal[K comparable, V any] struct {
	key K
	val V
}

type keyValErr[K comparable, V any] struct {
	key K
	val V
	err error
}

// GoMap returns a new map by transforming each element with the function
// fn, called by multiple go-routines.
func GoMap[IN_KEY comparable, IN_VAL any, OUT_KEY comparable, OUT_VAL any](in map[IN_KEY]IN_VAL, fn func(k IN_KEY, v IN_VAL) (OUT_KEY, OUT_VAL, error)) (map[OUT_KEY]OUT_VAL, error) {
	// Create a channel of input key-value pairs
	tasks := make(chan keyVal[IN_KEY, IN_VAL], 256)
	go func() {
		for k, v := range in {
			tasks <- keyVal[IN_KEY, IN_VAL]{k, v}
		}
		close(tasks)
	}()

	// Kick a number of workers to process the elements
	results := make(chan keyValErr[OUT_KEY, OUT_VAL], 256)
	go func() {
		numWorkers := runtime.NumCPU()
		wg := sync.WaitGroup{}
		wg.Add(numWorkers)
		for worker := 0; worker < numWorkers; worker++ {
			go func() {
				defer wg.Done()
				for task := range tasks {
					outKey, outValue, err := fn(task.key, task.val)
					results <- keyValErr[OUT_KEY, OUT_VAL]{
						key: outKey,
						val: outValue,
						err: err,
					}
				}
			}()
		}
		wg.Wait()
		close(results)
	}()

	out := make(map[OUT_KEY]OUT_VAL, len(in))
	errs := Errors{}
	for res := range results {
		if res.err != nil {
			errs = append(errs, res.err)
		} else {
			out[res.key] = res.val
		}
	}

	if len(errs) > 0 {
		return nil, errs
	}

	return out, nil
}
