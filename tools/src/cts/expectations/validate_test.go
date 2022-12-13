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

package expectations_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"github.com/google/go-cmp/cmp"
)

func TestValidate(t *testing.T) {
	header := `# BEGIN TAG HEADER
# OS
# tags: [ os-a os-b os-c ]
# GPU
# tags: [ gpu-a gpu-b gpu-c ]
# END TAG HEADER
`

	type Test struct {
		name         string
		expectations string
		diagnostics  expectations.Diagnostics
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			name:         "empty",
			expectations: ``,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "simple",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "no-tag collision",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Failure ]
crbug.com/a/123 a:x,x:d,* [ Failure ]
crbug.com/a/123 a:b,c:d,* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     8,
					Severity: expectations.Error,
					Message:  "expectation collides with expectation on line 10",
				},
				{
					Line:     10,
					Severity: expectations.Error,
					Message:  "expectation collides with expectation on line 8",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "tag collision",
			expectations: `
crbug.com/a/123 [ os-a ] a:b,c:d,* [ Failure ]
crbug.com/a/123 a:x,x:d,* [ Failure ]
crbug.com/a/123 [ os-a ] a:b,c:d,* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     8,
					Severity: expectations.Error,
					Message:  "expectation collides with expectation on line 10",
				},
				{
					Line:     10,
					Severity: expectations.Error,
					Message:  "expectation collides with expectation on line 8",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "nested no-tag collision",
			expectations: `
crbug.com/a/123 a:b,c:d,e:* [ Failure ]
crbug.com/a/123 a:x,x:d,* [ Failure ]
crbug.com/a/123 a:b,c:d,* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     10,
					Severity: expectations.Error,
					Message:  "expectation collides with expectation on line 8",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "tag collision",
			expectations: `
crbug.com/a/123 [ os-a ] a:b,c:d,e:* [ Failure ]
crbug.com/a/123 a:x,x:d,* [ Failure ]
crbug.com/a/123 [ os-a ] a:b,c:d,* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     10,
					Severity: expectations.Error,
					Message:  "expectation collides with expectation on line 8",
				},
			},
		},
	} {
		ex, err := expectations.Parse("expectations.txt", header+test.expectations)
		if err != nil {
			t.Fatalf("'%v': expectations.Parse():\n%v", test.name, err)
		}

		diagnostics := ex.Validate()
		if diff := cmp.Diff(diagnostics, test.diagnostics); diff != "" {
			t.Errorf("'%v': expectations.Update() error:\n%v", test.name, diff)
		}
	}
}
