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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/transform"
)

func TestMap(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.Map(in, func(k string, v int) (string, int, error) {
		return k + "+1", v + 1, nil
	})
	if e := check(out, map[string]int{"five+1": 6, "eight+1": 9, "two+1": 3, "four+1": 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestMapErr(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.Map(in, func(k string, v int) (string, int, error) {
		return "", 0, fmt.Errorf("%v:%v", k, v)
	})
	if e := check(out, map[string]int(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), `eight:8
five:5
four:4
two:2`); e != nil {
		t.Error(e)
	}
}

func TestGoMap(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.GoMap(in, func(k string, v int) (string, int, error) {
		return k + "+1", v + 1, nil
	})
	if e := check(out, map[string]int{"five+1": 6, "eight+1": 9, "two+1": 3, "four+1": 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestGoMapErr(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.GoMap(in, func(k string, v int) (string, int, error) {
		return "", 0, fmt.Errorf("%v:%v", k, v)
	})
	if e := check(out, map[string]int(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), `eight:8
five:5
four:4
two:2`); e != nil {
		t.Error(e)
	}
}
