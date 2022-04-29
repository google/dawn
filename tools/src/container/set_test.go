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
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
)

func TestNewEmptySet(t *testing.T) {
	s := container.NewSet[string]()
	expectEq(t, "len(s)", len(s), 0)
}

func TestNewSet(t *testing.T) {
	s := container.NewSet("c", "a", "b")
	expectEq(t, "len(s)", len(s), 3)
}

func TestSetList(t *testing.T) {
	s := container.NewSet("c", "a", "b")
	expectEq(t, "s.List()", s.List(), []string{"a", "b", "c"})
}

func TestSetClone(t *testing.T) {
	a := container.NewSet("c", "a", "b")
	b := a.Clone()
	a.Remove("a")
	expectEq(t, "b.List()", b.List(), []string{"a", "b", "c"})
}

func TestSetAdd(t *testing.T) {
	s := container.NewSet[string]()
	s.Add("c")
	expectEq(t, "len(s)", len(s), 1)
	expectEq(t, "s.List()", s.List(), []string{"c"})

	s.Add("a")
	expectEq(t, "len(s)", len(s), 2)
	expectEq(t, "s.List()", s.List(), []string{"a", "c"})

	s.Add("b")
	expectEq(t, "len(s)", len(s), 3)
	expectEq(t, "s.List()", s.List(), []string{"a", "b", "c"})
}

func TestSetRemove(t *testing.T) {
	s := container.NewSet("c", "a", "b")
	s.Remove("c")
	expectEq(t, "len(s)", len(s), 2)
	expectEq(t, "s.List()", s.List(), []string{"a", "b"})

	s.Remove("a")
	expectEq(t, "len(s)", len(s), 1)
	expectEq(t, "s.List()", s.List(), []string{"b"})

	s.Remove("b")
	expectEq(t, "len(s)", len(s), 0)
	expectEq(t, "s.List()", s.List(), []string{})
}

func TestSetContains(t *testing.T) {
	s := container.NewSet[string]()
	s.Add("c")
	expectEq(t, `m.Contains("a")`, s.Contains("a"), false)
	expectEq(t, `s.Contains("b")`, s.Contains("b"), false)
	expectEq(t, `s.Contains("c")`, s.Contains("c"), true)

	s.Add("a")
	expectEq(t, `s.Contains("a")`, s.Contains("a"), true)
	expectEq(t, `s.Contains("b")`, s.Contains("b"), false)
	expectEq(t, `s.Contains("c")`, s.Contains("c"), true)

	s.Add("b")
	expectEq(t, `s.Contains("a")`, s.Contains("a"), true)
	expectEq(t, `s.Contains("b")`, s.Contains("b"), true)
	expectEq(t, `s.Contains("c")`, s.Contains("c"), true)
}

func TestSetContainsAll(t *testing.T) {
	S := container.NewSet[string]

	s := container.NewSet[string]()
	s.Add("c")
	expectEq(t, `s.ContainsAll("a")`, s.ContainsAll(S("a")), false)
	expectEq(t, `s.ContainsAll("b")`, s.ContainsAll(S("b")), false)
	expectEq(t, `s.ContainsAll("c")`, s.ContainsAll(S("c")), true)
	expectEq(t, `s.ContainsAll("a", "b")`, s.ContainsAll(S("a", "b")), false)
	expectEq(t, `s.ContainsAll("b", "c")`, s.ContainsAll(S("b", "c")), false)
	expectEq(t, `s.ContainsAll("c", "a")`, s.ContainsAll(S("c", "a")), false)
	expectEq(t, `s.ContainsAll("c", "a", "b")`, s.ContainsAll(S("c", "a", "b")), false)

	s.Add("a")
	expectEq(t, `s.ContainsAll("a")`, s.ContainsAll(S("a")), true)
	expectEq(t, `s.ContainsAll("b")`, s.ContainsAll(S("b")), false)
	expectEq(t, `s.ContainsAll("c")`, s.ContainsAll(S("c")), true)
	expectEq(t, `s.ContainsAll("a", "b")`, s.ContainsAll(S("a", "b")), false)
	expectEq(t, `s.ContainsAll("b", "c")`, s.ContainsAll(S("b", "c")), false)
	expectEq(t, `s.ContainsAll("c", "a")`, s.ContainsAll(S("c", "a")), true)
	expectEq(t, `s.ContainsAll("c", "a", "b")`, s.ContainsAll(S("c", "a", "b")), false)

	s.Add("b")
	expectEq(t, `s.ContainsAll("a")`, s.ContainsAll(S("a")), true)
	expectEq(t, `s.ContainsAll("b")`, s.ContainsAll(S("b")), true)
	expectEq(t, `s.ContainsAll("c")`, s.ContainsAll(S("c")), true)
	expectEq(t, `s.ContainsAll("a", "b")`, s.ContainsAll(S("a", "b")), true)
	expectEq(t, `s.ContainsAll("b", "c")`, s.ContainsAll(S("b", "c")), true)
	expectEq(t, `s.ContainsAll("c", "a")`, s.ContainsAll(S("c", "a")), true)
	expectEq(t, `s.ContainsAll("c", "a", "b")`, s.ContainsAll(S("c", "a", "b")), true)
}

func TestSetContainsAny(t *testing.T) {
	S := container.NewSet[string]

	s := container.NewSet[string]()
	s.Add("c")
	expectEq(t, `s.ContainsAny("a")`, s.ContainsAny(S("a")), false)
	expectEq(t, `s.ContainsAny("b")`, s.ContainsAny(S("b")), false)
	expectEq(t, `s.ContainsAny("c")`, s.ContainsAny(S("c")), true)
	expectEq(t, `s.ContainsAny("a", "b")`, s.ContainsAny(S("a", "b")), false)
	expectEq(t, `s.ContainsAny("b", "c")`, s.ContainsAny(S("b", "c")), true)
	expectEq(t, `s.ContainsAny("c", "a")`, s.ContainsAny(S("c", "a")), true)
	expectEq(t, `s.ContainsAny("c", "a", "b")`, s.ContainsAny(S("c", "a", "b")), true)

	s.Add("a")
	expectEq(t, `s.ContainsAny("a")`, s.ContainsAny(S("a")), true)
	expectEq(t, `s.ContainsAny("b")`, s.ContainsAny(S("b")), false)
	expectEq(t, `s.ContainsAny("c")`, s.ContainsAny(S("c")), true)
	expectEq(t, `s.ContainsAny("a", "b")`, s.ContainsAny(S("a", "b")), true)
	expectEq(t, `s.ContainsAny("b", "c")`, s.ContainsAny(S("b", "c")), true)
	expectEq(t, `s.ContainsAny("c", "a")`, s.ContainsAny(S("c", "a")), true)
	expectEq(t, `s.ContainsAny("c", "a", "b")`, s.ContainsAny(S("c", "a", "b")), true)

	s.Remove("c")
	s.Add("b")
	expectEq(t, `s.ContainsAny("a")`, s.ContainsAny(S("a")), true)
	expectEq(t, `s.ContainsAny("b")`, s.ContainsAny(S("b")), true)
	expectEq(t, `s.ContainsAny("c")`, s.ContainsAny(S("c")), false)
	expectEq(t, `s.ContainsAny("a", "b")`, s.ContainsAny(S("a", "b")), true)
	expectEq(t, `s.ContainsAny("b", "c")`, s.ContainsAny(S("b", "c")), true)
	expectEq(t, `s.ContainsAny("c", "a")`, s.ContainsAny(S("c", "a")), true)
	expectEq(t, `s.ContainsAny("c", "a", "b")`, s.ContainsAny(S("c", "a", "b")), true)
}

func TestSetIntersection(t *testing.T) {
	a := container.NewSet(1, 3, 4, 6)
	b := container.NewSet(2, 3, 4, 5)

	i := a.Intersection(b)
	expectEq(t, `i.List()`, i.List(), []int{3, 4})
}

func TestSetAddAll(t *testing.T) {
	s := container.NewSet[string]()
	s.AddAll(container.NewSet("c", "a"))
	expectEq(t, "len(s)", len(s), 2)
	expectEq(t, "s.List()", s.List(), []string{"a", "c"})
}

func TestSetRemoveAll(t *testing.T) {
	s := container.NewSet("c", "a", "b")
	s.RemoveAll(container.NewSet("c", "a"))
	expectEq(t, "len(s)", len(s), 1)
	expectEq(t, "s.List()", s.List(), []string{"b"})
}

func TestSetOne(t *testing.T) {
	expectEq(t, "NewSet[string]().One()", container.NewSet[string]().One(), "")
	expectEq(t, `NewSet("x").One()`, container.NewSet("x").One(), "x")
	if got := container.NewSet("x", "y").One(); got != "x" && got != "y" {
		t.Errorf(`NewSet("x", "y").One() returned "%v"`, got)
	}
}

func TestFormat(t *testing.T) {
	expectEq(t, "NewSet[string]()", fmt.Sprint(container.NewSet[string]()), "[]")
	expectEq(t, `NewSet("x")`, fmt.Sprint(container.NewSet("x")), `[x]`)
	expectEq(t, `NewSet(1)`, fmt.Sprint(container.NewSet(1)), `[1]`)
	expectEq(t, `NewSet("y", "x")`, fmt.Sprint(container.NewSet("y", "x")), `[x, y]`)
	expectEq(t, `NewSet(3, 1, 2)`, fmt.Sprint(container.NewSet(3, 1, 2)), `[1, 2, 3]`)
}
