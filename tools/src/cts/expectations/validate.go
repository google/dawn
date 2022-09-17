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

// Package expectations provides types and helpers for parsing, updating and
// writing WebGPU expectations files.
//
// See <dawn>/webgpu-cts/expectations.txt for more information.
package expectations

import (
	"fmt"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
)

func (c Content) tagsCollide(a, b container.Set[string]) bool {
	for _, set := range c.Tags.Sets {
		aSet := a.Intersection(set.Tags)
		bSet := b.Intersection(set.Tags)
		if len(aSet) != 0 && len(bSet) != 0 && len(aSet.Intersection(bSet)) == 0 {
			return false
		}
	}
	return true
}

// Validate checks that the expectations do not contain errors
func (c Content) Validate() Diagnostics {
	tree, _ := query.NewTree[Expectations]()
	for _, chunk := range c.Chunks {
		for _, ex := range chunk.Expectations {
			node := tree.GetOrCreate(query.Parse(ex.Query), func() Expectations {
				return Expectations{}
			})
			*node = append(*node, ex)
		}
	}
	var out Diagnostics
	for _, chunk := range c.Chunks {
		for _, ex := range chunk.Expectations {
			glob, err := tree.Glob(query.Parse(ex.Query))
			if err != nil {
				out = append(out, Diagnostic{
					Severity: Error,
					Line:     ex.Line,
					Message:  err.Error(),
				})
				continue
			}
			for _, qd := range glob {
				expectations := qd.Data
				for _, other := range expectations {
					if other.Line != ex.Line && c.tagsCollide(ex.Tags, other.Tags) {
						out = append(out, Diagnostic{
							Severity: Error,
							Line:     ex.Line,
							Message:  fmt.Sprintf("expectation collides with expectation on line %v", other.Line),
						})
					}
				}
			}
		}
	}
	return out
}
