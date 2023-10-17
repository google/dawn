// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package expectations_test

import (
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"github.com/google/go-cmp/cmp"
)

var Q = query.Parse

func TestUpdate(t *testing.T) {
	header := `# BEGIN TAG HEADER
# OS
# tags: [ os-a os-b os-c ]
# GPU
# tags: [ gpu-a gpu-b gpu-c ]
# END TAG HEADER
`
	headerLines := strings.Count(header, "\n")

	type Test struct {
		name         string
		expectations string
		results      result.List
		updated      string
		diagnostics  expectations.Diagnostics
		err          string
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			name:         "empty results",
			expectations: ``,
			results:      result.List{},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "no results found",
			expectations: `
crbug.com/a/123 a:missing,test,result:* [ Failure ]
crbug.com/a/123 [ tag ] another:missing,test,result:* [ Failure ]

some:other,test:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("some:other,test:*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("some:other,test:*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Failure,
				},
			},
			updated: `
some:other,test:* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Severity: expectations.Warning,
					Line:     headerLines + 2,
					Message:  "no results found for 'a:missing,test,result:*'",
				},
				{
					Severity: expectations.Warning,
					Line:     headerLines + 3,
					Message:  "no results found for 'another:missing,test,result:*' with tags [tag]",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "no results found KEEP",
			expectations: `
# KEEP
crbug.com/a/123 a:missing,test,result:* [ Failure ]

some:other,test:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("some:other,test:*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("some:other,test:*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Failure,
				},
			},
			updated: `
some:other,test:* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Severity: expectations.Warning,
					Line:     headerLines + 3,
					Message:  "no results found for 'a:missing,test,result:*'",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "simple expectation with tags",
			expectations: `
[ os-a ] a:b,c:* [ Failure ]
[ gpu-b ] a:b,c:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d"),
					Tags:   result.NewTags("os-a", "os-c", "gpu-b"),
					Status: result.Failure,
				},
			},
			updated: `
a:b,c:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation test now passes",
			expectations: `
crbug.com/a/123 [ gpu-a os-a ] a:b,c:* [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Abort,
				},
			},
			updated: `
crbug.com/a/123 [ os-b ] a:b,c:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation case now passes",
			expectations: `
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:d [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Abort,
				},
			},
			updated: `
crbug.com/a/123 [ os-b ] a:b,c:d: [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation case now passes KEEP - single",
			expectations: `
# KEEP
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:d [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Abort,
				},
			},
			updated: `
# KEEP
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:d [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Severity: expectations.Note,
					Line:     headerLines + 3,
					Message:  "test now passes",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation case now passes KEEP - multiple",
			expectations: `
# KEEP
crbug.com/a/123 a:b,c:d:* [ Failure ]
`,
			results: result.List{
				result.Result{Query: Q("a:b,c:d:a"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:d:b"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:d:c"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:d:d"), Status: result.Pass},
			},
			updated: `
# KEEP
crbug.com/a/123 a:b,c:d:* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Severity: expectations.Note,
					Line:     headerLines + 3,
					Message:  "all 4 tests now pass",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "new test results",
			expectations: `# A comment`,
			results: result.List{
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_a:*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Abort,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_a:*"),
					Tags:   result.NewTags("os-a", "gpu-b"),
					Status: result.Abort,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_c:case=4;*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Crash,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_c:case=4;*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Crash,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_c:case=5;*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.RetryOnFailure,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_b:case=5;*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_b:case=6;*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Slow,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_b:case=6;*"),
					Tags:   result.NewTags("os-b", "gpu-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("suite:dir_a,dir_b:test_c:case=6;*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.RetryOnFailure,
				},
			},
			updated: `# A comment

################################################################################
# New flakes. Please triage:
################################################################################
crbug.com/dawn/0000 suite:dir_a,dir_b:test_c:case=5;* [ RetryOnFailure ]
crbug.com/dawn/0000 suite:dir_a,dir_b:test_c:case=6;* [ RetryOnFailure ]

################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 suite:dir_a,dir_b:test_a:* [ Failure ]
crbug.com/dawn/0000 [ gpu-a os-a ] suite:dir_a,dir_b:test_b:* [ Slow ]
crbug.com/dawn/0000 suite:dir_a,dir_b:test_c:case=4;* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "filter unknown tags",
			expectations: ``,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-a", "gpu-x"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-b", "gpu-x"),
					Status: result.Crash,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-x", "gpu-b"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-x", "gpu-a"),
					Status: result.Crash,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-c", "gpu-c"),
					Status: result.Pass,
				},
			},
			updated: `
################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 [ gpu-a ] a:* [ Failure ]
crbug.com/dawn/0000 [ gpu-b ] a:* [ Failure ]
crbug.com/dawn/0000 [ os-a ] a:* [ Failure ]
crbug.com/dawn/0000 [ os-b ] a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "prioritize tag sets",
			expectations: ``,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-a", "os-c", "gpu-b"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("gpu-a", "os-b", "gpu-c"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:*"),
					Tags:   result.NewTags("os-c", "gpu-c"),
					Status: result.Pass,
				},
			},
			updated: `
################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 [ gpu-b os-c ] a:* [ Failure ]
crbug.com/dawn/0000 [ gpu-c os-b ] a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "merge when 50% or more children fail",
			expectations: ``,
			results: result.List{ // 4 pass, 6 fail (50%)
				result.Result{Query: Q("a:b,c:0:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:1:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:2:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:3:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:4:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:5:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:6:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:7:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:8:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:9:*"), Status: result.Pass},
			},
			updated: `
################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "don't merge when 50% or fewer children fail",
			expectations: ``,
			results: result.List{ // 5 pass, 5 fail (50%)
				result.Result{Query: Q("a:b,c:0:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:1:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:2:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:3:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:4:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:5:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:6:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:7:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:8:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:9:*"), Status: result.Pass},
			},
			updated: `
################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 a:b,c:0:* [ Failure ]
crbug.com/dawn/0000 a:b,c:2:* [ Failure ]
crbug.com/dawn/0000 a:b,c:5:* [ Failure ]
crbug.com/dawn/0000 a:b,c:6:* [ Failure ]
crbug.com/dawn/0000 a:b,c:8:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "merge when more than 10 children fail",
			expectations: ``,
			results: result.List{ // 19 pass, 11 fail (37%)
				result.Result{Query: Q("a:b,c:00:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:01:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:02:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:03:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:04:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:05:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:06:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:07:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:08:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:09:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:10:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:11:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:12:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:13:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:14:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:15:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:16:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:17:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:18:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:19:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:20:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:21:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:22:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:23:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:24:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:25:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:26:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:27:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:28:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:29:*"), Status: result.Failure},
			},
			updated: `
################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "don't merge when 10 or fewer children fail",
			expectations: ``,
			results: result.List{ // 20 pass, 10 fail (33%)
				result.Result{Query: Q("a:b,c:00:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:01:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:02:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:03:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:04:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:05:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:06:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:07:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:08:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:09:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:10:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:11:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:12:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:13:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:14:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:15:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:16:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:17:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:18:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:19:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:20:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:21:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:22:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:23:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:24:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:25:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:26:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:27:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:28:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:29:*"), Status: result.Failure},
			},
			updated: `
################################################################################
# New failures. Please triage:
################################################################################
crbug.com/dawn/0000 a:b,c:00:* [ Failure ]
crbug.com/dawn/0000 a:b,c:05:* [ Failure ]
crbug.com/dawn/0000 a:b,c:08:* [ Failure ]
crbug.com/dawn/0000 a:b,c:13:* [ Failure ]
crbug.com/dawn/0000 a:b,c:15:* [ Failure ]
crbug.com/dawn/0000 a:b,c:20:* [ Failure ]
crbug.com/dawn/0000 a:b,c:23:* [ Failure ]
crbug.com/dawn/0000 a:b,c:26:* [ Failure ]
crbug.com/dawn/0000 a:b,c:28:* [ Failure ]
crbug.com/dawn/0000 a:b,c:29:* [ Failure ]
`,
		},
	} {
		ex, err := expectations.Parse("expectations.txt", header+test.expectations)
		if err != nil {
			t.Fatalf("'%v': expectations.Parse():\n%v", test.name, err)
		}

		testList := container.NewMap[string, query.Query]()
		for _, r := range test.results {
			testList.Add(r.Query.String(), r.Query)
		}

		errMsg := ""
		diagnostics, err := ex.Update(test.results, testList.Values())
		if err != nil {
			errMsg = err.Error()
		}
		if diff := cmp.Diff(errMsg, test.err); diff != "" {
			t.Errorf("'%v': expectations.Update() error:\n%v", test.name, diff)
		}

		if diff := cmp.Diff(diagnostics, test.diagnostics); diff != "" {
			t.Errorf("'%v': diagnostics were not as expected:\n%v", test.name, diff)
		}

		if diff := cmp.Diff(
			strings.Split(ex.String(), "\n"),
			strings.Split(header+test.updated, "\n")); diff != "" {
			t.Errorf("'%v': updated was not as expected:\n%v", test.name, diff)
		}
	}
}
