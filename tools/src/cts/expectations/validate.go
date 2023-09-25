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

	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"github.com/google/go-cmp/cmp"
)

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
			for _, status := range ex.Status {
				if status == "Slow" {
					out = append(out, Diagnostic{
						Severity: Error,
						Line:     ex.Line,
						Message:  fmt.Sprintf("\"Slow\" expectation is not valid here. Use slow_tests.txt instead."),
					})
				}
			}
			_, err := tree.Glob(query.Parse(ex.Query))
			if err != nil {
				out = append(out, Diagnostic{
					Severity: Error,
					Line:     ex.Line,
					Message:  err.Error(),
				})
				continue
			}
		}
	}
	return out
}

// ValidateSlowTests checks that the expectations are only [ Slow ]
func (c Content) ValidateSlowTests() Diagnostics {
	var out Diagnostics
	for _, chunk := range c.Chunks {
		for _, ex := range chunk.Expectations {
			if !cmp.Equal(ex.Status, []string{"Slow"}) {
				out = append(out, Diagnostic{
					Severity: Error,
					Line:     ex.Line,
					Message:  fmt.Sprintf("slow test expectation for %v must be %v but was %v", ex.Query, []string{"Slow"}, ex.Status),
				})
			}
		}
	}
	return out
}
