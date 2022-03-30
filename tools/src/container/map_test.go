// Copyright 2022 The Dawn Authors
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

package container_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
)

func TestNewMap(t *testing.T) {
	m := container.NewMap[string, int]()
	expectEq(t, "len(m)", len(m), 0)
}

func TestMapAdd(t *testing.T) {
	m := container.NewMap[string, int]()
	m.Add("c", 3)
	expectEq(t, "len(m)", len(m), 1)
	expectEq(t, `m["a"]`, m["a"], 0)
	expectEq(t, `m["b"]`, m["b"], 0)
	expectEq(t, `m["c"]`, m["c"], 3)

	m.Add("a", 1)
	expectEq(t, "len(m)", len(m), 2)
	expectEq(t, `m["a"]`, m["a"], 1)
	expectEq(t, `m["b"]`, m["b"], 0)
	expectEq(t, `m["c"]`, m["c"], 3)

	m.Add("b", 2)
	expectEq(t, "len(m)", len(m), 3)
	expectEq(t, `m["a"]`, m["a"], 1)
	expectEq(t, `m["b"]`, m["b"], 2)
	expectEq(t, `m["c"]`, m["c"], 3)
}

func TestMapRemove(t *testing.T) {
	m := container.NewMap[string, int]()
	m.Add("a", 1)
	m.Add("b", 2)
	m.Add("c", 3)

	m.Remove("c")
	expectEq(t, "len(m)", len(m), 2)
	expectEq(t, `m["a"]`, m["a"], 1)
	expectEq(t, `m["b"]`, m["b"], 2)
	expectEq(t, `m["c"]`, m["c"], 0)

	m.Remove("a")
	expectEq(t, "len(m)", len(m), 1)
	expectEq(t, `m["a"]`, m["a"], 0)
	expectEq(t, `m["b"]`, m["b"], 2)
	expectEq(t, `m["c"]`, m["c"], 0)

	m.Remove("b")
	expectEq(t, "len(m)", len(m), 0)
	expectEq(t, `m["a"]`, m["a"], 0)
	expectEq(t, `m["b"]`, m["b"], 0)
	expectEq(t, `m["c"]`, m["c"], 0)
}

func TestMapContains(t *testing.T) {
	m := container.NewMap[string, int]()
	m.Add("c", 3)
	expectEq(t, `m.Contains("a")`, m.Contains("a"), false)
	expectEq(t, `m.Contains("b")`, m.Contains("b"), false)
	expectEq(t, `m.Contains("c")`, m.Contains("c"), true)

	m.Add("a", 1)
	expectEq(t, `m.Contains("a")`, m.Contains("a"), true)
	expectEq(t, `m.Contains("b")`, m.Contains("b"), false)
	expectEq(t, `m.Contains("c")`, m.Contains("c"), true)

	m.Add("b", 2)
	expectEq(t, `m.Contains("a")`, m.Contains("a"), true)
	expectEq(t, `m.Contains("b")`, m.Contains("b"), true)
	expectEq(t, `m.Contains("c")`, m.Contains("c"), true)
}

func TestMapKeys(t *testing.T) {
	m := container.NewMap[string, int]()
	m.Add("c", 3)
	expectEq(t, `m.Keys()`, m.Keys(), []string{"c"})

	m.Add("a", 1)
	expectEq(t, `m.Keys()`, m.Keys(), []string{"a", "c"})

	m.Add("b", 2)
	expectEq(t, `m.Keys()`, m.Keys(), []string{"a", "b", "c"})
}

func TestMapValues(t *testing.T) {
	m := container.NewMap[string, int]()
	m.Add("c", 1)
	expectEq(t, `m.Values()`, m.Values(), []int{1})

	m.Add("a", 2)
	expectEq(t, `m.Values()`, m.Values(), []int{2, 1})

	m.Add("b", 3)
	expectEq(t, `m.Values()`, m.Values(), []int{2, 3, 1})
}
