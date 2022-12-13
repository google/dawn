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

# New flakes. Please triage:
crbug.com/dawn/0000 suite:dir_a,dir_b:test_c:case=5;* [ RetryOnFailure ]
crbug.com/dawn/0000 suite:dir_a,dir_b:test_c:case=6;* [ RetryOnFailure ]

# New failures. Please triage:
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
# New failures. Please triage:
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
# New failures. Please triage:
crbug.com/dawn/0000 [ gpu-b os-c ] a:* [ Failure ]
crbug.com/dawn/0000 [ gpu-c os-b ] a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "merge when over 75% of children fail",
			expectations: ``,
			results: result.List{
				result.Result{Query: Q("a:b,c:t0:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t1:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t2:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t3:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t4:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t5:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t6:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t7:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t8:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t9:*"), Status: result.Failure},
			},
			updated: `
# New failures. Please triage:
crbug.com/dawn/0000 a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "don't merge when under 75% of children fail",
			expectations: ``,
			results: result.List{
				result.Result{Query: Q("a:b,c:t0:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t1:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t2:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t3:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t4:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t5:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t6:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t7:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t8:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t9:*"), Status: result.Failure},
			},
			updated: `
# New failures. Please triage:
crbug.com/dawn/0000 a:b,c:t0:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t2:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t3:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t5:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t6:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t8:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t9:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "merge when over 20 children fail",
			expectations: ``,
			results: result.List{ // 21 failures, 70% fail
				result.Result{Query: Q("a:b,c:t00:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t01:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t02:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t03:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t04:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t05:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t06:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t07:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t08:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t09:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t10:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t11:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t12:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t13:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t14:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t15:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t16:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t17:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t18:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t19:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t20:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t21:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t22:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t23:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t24:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t25:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t26:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t27:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t28:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t29:*"), Status: result.Failure},
			},
			updated: `
# New failures. Please triage:
crbug.com/dawn/0000 a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name:         "dont merge when under 21 children fail",
			expectations: ``,
			results: result.List{ // 20 failures, 66% fail
				result.Result{Query: Q("a:b,c:t00:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t01:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t02:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t03:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t04:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t05:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t06:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t07:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t08:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t09:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t10:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t11:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t12:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t13:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t14:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t15:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t16:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t17:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t18:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t19:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t20:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t21:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t22:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t23:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t24:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t25:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t26:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t27:*"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:t28:*"), Status: result.Failure},
				result.Result{Query: Q("a:b,c:t29:*"), Status: result.Failure},
			},
			updated: `
# New failures. Please triage:
crbug.com/dawn/0000 a:b,c:t00:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t02:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t04:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t05:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t06:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t08:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t09:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t10:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t13:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t15:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t16:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t18:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t19:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t20:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t22:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t23:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t25:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t26:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t28:* [ Failure ]
crbug.com/dawn/0000 a:b,c:t29:* [ Failure ]
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
