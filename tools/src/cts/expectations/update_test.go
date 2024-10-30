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

package expectations

import (
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"github.com/google/go-cmp/cmp"
	"github.com/stretchr/testify/assert"
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
		diagnostics  Diagnostics
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
# ##ROLLER_MUTABLE##
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
# ##ROLLER_MUTABLE##
some:other,test:* [ Failure ]
crbug.com/a/123 a:missing,test,result:* [ Failure ]
crbug.com/a/123 [ tag ] another:missing,test,result:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 3,
					Message:  "no results found for query 'a:missing,test,result:*'",
				},
				{
					Severity: Note,
					Line:     headerLines + 4,
					Message:  "no results found for query 'another:missing,test,result:*' with tags [tag]",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "no results found immutable",
			expectations: `
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
crbug.com/a/123 a:missing,test,result:* [ Failure ]

some:other,test:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 2,
					Message:  "no results found for query 'a:missing,test,result:*'",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "unknown test",
			expectations: `
# ##ROLLER_MUTABLE##
crbug.com/a/123 an:unknown,test:* [ Failure ]
crbug.com/a/123 [ tag ] another:unknown:test [ Failure ]

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
			diagnostics: Diagnostics{
				{
					Severity: Warning,
					Line:     headerLines + 3,
					Message:  "no tests exist with query 'an:unknown,test:*' - removing",
				},
				{
					Severity: Warning,
					Line:     headerLines + 4,
					Message:  "no tests exist with query 'another:unknown:test' - removing",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "unknown test found in immutable chunk",
			expectations: `
crbug.com/a/123 an:unknown,test:* [ Failure ]

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
			diagnostics: Diagnostics{
				{
					Severity: Warning,
					Line:     headerLines + 2,
					Message:  "no tests exist with query 'an:unknown,test:*' - removing",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "simple expectation with tags",
			expectations: `
# ##ROLLER_MUTABLE##
[ os-a ] a:b,c:* [ Failure ]
[ gpu-b ] a:b,c:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("os-a", "os-c", "gpu-b"),
					Status: result.Failure,
				},
			},
			updated: `
# ##ROLLER_MUTABLE##
a:b,c:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 4,
					Message:  "expectation is fully covered by previous expectations",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "simple expectation with tags new flakes implicitly mutable",
			expectations: `
################################################################################
# New flakes. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
[ os-a ] a:b,c:* [ RetryOnFailure ]
[ gpu-b ] a:b,c:* [ RetryOnFailure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("os-a", "os-c", "gpu-b"),
					Status: result.RetryOnFailure,
				},
			},
			updated: `
################################################################################
# New flakes. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
crbug.com/dawn/0000 a:* [ RetryOnFailure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "simple expectation with tags new failures implicitly mutable",
			expectations: `
################################################################################
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
[ os-a ] a:b,c:* [ Failure ]
[ gpu-b ] a:b,c:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("os-a", "os-c", "gpu-b"),
					Status: result.Failure,
				},
			},
			updated: `
################################################################################
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
crbug.com/dawn/0000 a:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "simple expectation with tags immutable",
			expectations: `
[ os-a ] a:b,c:* [ Failure ]
[ gpu-b ] a:b,c:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("os-a", "os-c", "gpu-b"),
					Status: result.Failure,
				},
			},
			updated: `
[ gpu-b ] a:b,c:* [ Failure ]
[ os-a ] a:b,c:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation test now passes",
			expectations: `
# ##ROLLER_MUTABLE##
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
# ##ROLLER_MUTABLE##
crbug.com/a/123 [ os-b ] a:b,c:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 4,
					Message:  "expectation is fully covered by previous expectations",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation case now passes",
			expectations: `
# ##ROLLER_MUTABLE##
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d:* [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:d:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:*"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d:*"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Abort,
				},
			},
			updated: `
# ##ROLLER_MUTABLE##
crbug.com/a/123 [ os-b ] a:b,c:d:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 4,
					Message:  "expectation is fully covered by previous expectations",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "first expectation expands to cover later expectations - no diagnostics",
			expectations: `
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d:* [ Failure ]
crbug.com/a/123 [ gpu-c os-a ] a:b,c:d:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("gpu-a", "os-a"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("gpu-a", "os-b"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("gpu-b", "os-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("gpu-b", "os-b"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("gpu-c", "os-a"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("gpu-c", "os-b"),
					Status: result.Pass,
				},
			},
			updated: `
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d:* [ Failure ]
crbug.com/a/123 [ gpu-c os-a ] a:b,c:d:* [ Failure ]
`,
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation case now passes immutable - single",
			expectations: `
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d:* [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:d:* [ Failure ]
`,
			results: result.List{
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("os-a", "gpu-a"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:d:e"),
					Tags:   result.NewTags("os-b", "gpu-b"),
					Status: result.Abort,
				},
			},
			updated: `
crbug.com/a/123 [ gpu-a os-a ] a:b,c:d:* [ Failure ]
crbug.com/a/123 [ gpu-b os-b ] a:b,c:d:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 2,
					Message:  "test now passes",
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			name: "expectation case now passes immutable - multiple",
			expectations: `
crbug.com/a/123 a:b,c:d:* [ Failure ]
`,
			results: result.List{
				result.Result{Query: Q("a:b,c:d:a"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:d:b"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:d:c"), Status: result.Pass},
				result.Result{Query: Q("a:b,c:d:d"), Status: result.Pass},
			},
			updated: `
crbug.com/a/123 a:b,c:d:* [ Failure ]
`,
			diagnostics: Diagnostics{
				{
					Severity: Note,
					Line:     headerLines + 2,
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
# New flakes. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
crbug.com/dawn/0000 [ gpu-a os-a ] suite:dir_a,dir_b:test_c:case=6;* [ RetryOnFailure ]
crbug.com/dawn/0000 [ gpu-b os-b ] suite:dir_a,dir_b:test_c:case=5;* [ RetryOnFailure ]

################################################################################
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
crbug.com/dawn/0000 [ gpu-a os-a ] suite:dir_a,dir_b:test_a:* [ Failure ]
crbug.com/dawn/0000 [ gpu-a os-a ] suite:dir_a,dir_b:test_b:* [ Slow ]
crbug.com/dawn/0000 [ gpu-a os-a ] suite:dir_a,dir_b:test_c:case=4;* [ Failure ]
crbug.com/dawn/0000 [ gpu-b os-a ] suite:* [ Failure ]
crbug.com/dawn/0000 [ gpu-b os-b ] suite:dir_a,dir_b:test_c:case=4;* [ Failure ]
`,
		},

		{ //////////////////////////////////////////////////////////////////////
			name:         "root node overlap",
			expectations: `# A comment`,
			results: result.List{
				// For variant ['os-a'], we have a root node 'a:b,c:d:*'.
				result.Result{
					Query:  Q("a:b,c:d:x,*"),
					Tags:   result.NewTags("os-a"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:d:y,*"),
					Tags:   result.NewTags("os-a"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:e:*"),
					Tags:   result.NewTags("os-a"),
					Status: result.Pass,
				},
				// For variant ['os-b'], we have a root node 'a:b,c:d:x,*'.
				result.Result{
					Query:  Q("a:b,c:d:x,*"),
					Tags:   result.NewTags("os-b"),
					Status: result.Failure,
				},
				result.Result{
					Query:  Q("a:b,c:d:y,*"),
					Tags:   result.NewTags("os-b"),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q("a:b,c:e:*"),
					Tags:   result.NewTags("os-b"),
					Status: result.Pass,
				},
			},
			updated: `# A comment

################################################################################
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
################################################################################
crbug.com/dawn/0000 [ os-a ] a:b,c:d:* [ Failure ]
crbug.com/dawn/0000 [ os-b ] a:b,c:d:x,* [ Failure ]
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
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
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
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
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
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
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
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
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
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
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
# New failures. Please triage - will be discarded/regenerated by the next roll:
# ##ROLLER_DISCARD_AND_REWRITE##
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
		ex, err := Parse("expectations.txt", header+test.expectations)
		if err != nil {
			t.Fatalf("'%v': expectations.Parse():\n%v", test.name, err)
		}

		testList := container.NewMap[string, query.Query]()
		for _, r := range test.results {
			testList.Add(r.Query.String(), r.Query)
		}
		for _, s := range []string{
			"a:missing,test,result:a=1,b=2",
			"another:missing,test,result:cat=meow,dog=woof",
		} {
			testList.Add(s, query.Parse(s))
		}

		errMsg := ""
		diagnostics, err := ex.Update(test.results, testList.Values() /* verbose */, false)
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

func createGenericUpdater(t *testing.T) updater {
	header := `
# BEGIN TAG HEADER
# OS
# tags: [ linux win10 ]
# GPU
# tags: [ intel
#         nvidia nvidia-0x2184 ]
# Driver
# tags: [ nvidia_ge_31.0.15.4601 nvidia_lt_31.0.15.4601
#         nvidia_ge_535.183.01 nvidia_lt_535.183.01 ]
# END TAG HEADER
`
	inContent, err := Parse("expectations.txt", header)
	if err != nil {
		t.Fatalf("Failed to parse expectations: %v", err)
	}

	u := updater{
		in: inContent,
	}
	return u
}

// Tests basic result -> expectation conversion.
func TestResultsToExpectationsBasic(t *testing.T) {
	results := result.List{
		{
			Query:  query.Parse("webgpu:shader,execution,memory_layout:read_layout:"),
			Tags:   result.NewTags("linux", "nvidia"),
			Status: result.Failure,
		},
		{
			Query:  query.Parse("webgpu:shader,execution,memory_model,barrier:"),
			Tags:   result.NewTags("win10", "intel"),
			Status: result.Failure,
		},
	}

	expectedOutput := []Expectation{
		{
			Bug:     "crbug.com/1234",
			Tags:    result.NewTags("linux", "nvidia"),
			Query:   "webgpu:shader,execution,memory_layout:read_layout:",
			Status:  []string{"Failure"},
			Comment: "comment",
		},
		{
			Bug:     "crbug.com/1234",
			Tags:    result.NewTags("win10", "intel"),
			Query:   "webgpu:shader,execution,memory_model,barrier",
			Status:  []string{"Failure"},
			Comment: "comment",
		},
	}

	u := createGenericUpdater(t)
	output := u.resultsToExpectations(results, "crbug.com/1234", "comment")
	assert.Equal(t, output, expectedOutput)
}

// Tests behavior when two unique results end up creating the same expectation
// due to lower priority tags being removed.
func TestResultsToExpectationsOverlappingExpectations(t *testing.T) {
	results := result.List{
		{
			Query:  query.Parse("webgpu:shader,execution,memory_layout:read_layout:"),
			Tags:   result.NewTags("nvidia", "nvidia-0x2184", "nvidia_ge_31.0.15.4601", "nvidia_lt_535.183.01"),
			Status: result.Failure,
		},
		{
			Query:  query.Parse("webgpu:shader,execution,memory_layout:read_layout:"),
			Tags:   result.NewTags("nvidia", "nvidia-0x2184", "nvidia_lt_31.0.15.4601", "nvidia_lt_535.183.01"),
			Status: result.Failure,
		},
	}

	expectedOutput := []Expectation{
		{
			Bug:     "crbug.com/1234",
			Tags:    result.NewTags("nvidia-0x2184", "nvidia_lt_535.183.01"),
			Query:   "webgpu:shader,execution,memory_layout:read_layout:",
			Status:  []string{"Failure"},
			Comment: "comment",
		},
	}

	u := createGenericUpdater(t)
	output := u.resultsToExpectations(results, "crbug.com/1234", "comment")
	assert.Equal(t, output, expectedOutput)
}

// Tests behavior related to automatic inclusion of a trailing :.
func TestResultsToExpectationsTrailingColon(t *testing.T) {
	results := result.List{
		// Should automatically have a : added since it's a test query.
		{
			Query:  query.Parse("webgpu:shader,execution,memory_layout:read_layout"),
			Tags:   result.NewTags("linux", "nvidia"),
			Status: result.Failure,
		},
		// Should not have a : added since it is a wildcard query.
		{
			Query:  query.Parse("webgpu:shader,execution,*"),
			Tags:   result.NewTags("win10", "intel"),
			Status: result.Failure,
		},
	}

	expectedOutput := []Expectation{
		{
			Bug:     "crbug.com/1234",
			Tags:    result.NewTags("win10", "intel"),
			Query:   "webgpu:shader,execution,*",
			Status:  []string{"Failure"},
			Comment: "comment",
		},
		{
			Bug:     "crbug.com/1234",
			Tags:    result.NewTags("linux", "nvidia"),
			Query:   "webgpu:shader,execution,memory_layout:read_layout:",
			Status:  []string{"Failure"},
			Comment: "comment",
		},
	}

	u := createGenericUpdater(t)
	output := u.resultsToExpectations(results, "crbug.com/1234", "comment")
	assert.Equal(t, output, expectedOutput)
}

/*******************************************************************************
 * removeExpectationsForUnknownTests tests
 ******************************************************************************/

// Tests that expectations for unknown tests are properly removed, even if they
// are in an immutable chunk.
func TestRemoveExpectationsForUnknownTests(t *testing.T) {
	startingContent := `
# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] a:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] invalid_test [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Fully removed, immutable.
[ android ] invalid_test [ Failure ]
[ android ] c:* [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] invalid_test [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", startingContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	knownTests := []query.Query{
		query.Parse("valid_test1"),
		query.Parse("valid_test2"),
		query.Parse("a:b"),
		query.Parse("a:b,c:d,e:f=g"),
	}

	content.removeExpectationsForUnknownTests(&knownTests)

	expectedContent := `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] a:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	assert.Equal(t, expectedContent, content.String())
}

/*******************************************************************************
 * removeUnknownTags tests
 ******************************************************************************/

// Tests that unknown tags are properly removed.
func TestRemoveUnknownTags(t *testing.T) {
	header := `# BEGIN TAG HEADER
# OS
# tags: [ android linux win10 ]
# END TAG HEADER
`

	content, err := Parse("expectations.txt", header)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	resultList := result.List{
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("android", "android-14", "arm"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("win", "win10", "intel", "intel-gen-9", "intel-0x3e9b"),
			Status: result.Failure,
		},
	}

	content.removeUnknownTags(&resultList)

	expectedList := result.List{
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("android"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("win10"),
			Status: result.Failure,
		},
	}

	assert.Equal(t, expectedList, resultList)
}

/*******************************************************************************
 * reduceTagsToMostExplicitOnly tests
 ******************************************************************************/

// Tests that result tags are properly filtered to only include the most
// explicit tags based on the tag sets in the header.
func TestReduceTagsToMostExplicitOnly(t *testing.T) {
	header := `
# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 win11 ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         imagination
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#             intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184 nvidia-0x2783
#         qualcomm qualcomm-0x41333430 qualcomm-0x36333630 qualcomm-0x36334330 ]
# END TAG HEADER
`

	content, err := Parse("expectations.txt", header)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	resultList := result.List{
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("android", "android-14", "arm"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("win", "win10", "intel", "intel-gen-9", "intel-0x3e9b"),
			Status: result.Failure,
		},
	}

	content.reduceTagsToMostExplicitOnly(&resultList)

	expectedList := result.List{
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("android-14", "arm"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("win10", "intel-0x3e9b"),
			Status: result.Failure,
		},
	}

	assert.Equal(t, expectedList, resultList)
}

/*******************************************************************************
 * addExpectationsToMutableChunk tests
 ******************************************************************************/

// Tests that multiple mutable chunks result in an error.
func TestAddExpectationsToMutableChunkMultipleMutableChunks(t *testing.T) {
	fileContent := `
# BEGIN TAG HEADER
# OS
# tags: [ android linux win10 ]
# END TAG HEADER

# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]

# ##ROLLER_AUTOGENERATED_FAILURES##
[ linux ] valid_test1 [ Failure ]
[ linux ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", fileContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	resultList := result.List{
		result.Result{
			Query:  query.Parse("test"),
			Tags:   result.NewTags("android-14", "arm"),
			Status: result.Failure,
		},
	}

	err = content.addExpectationsToMutableChunk(&resultList)
	assert.EqualError(t, err, "Expected 1 mutable chunk, found 2")
}

// Tests that a single mutable chunk gets expectations added to it.
func TestAddExpectationsToMutableChunkSingleMutableChunk(t *testing.T) {
	fileContent := `
# BEGIN TAG HEADER
# OS
# tags: [ android linux win10 ]
# END TAG HEADER

# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", fileContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	resultList := result.List{
		result.Result{
			Query:  query.Parse("a:b,c:d"),
			Tags:   result.NewTags("android-14", "arm"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("z:b,c:d,e:f=g"),
			Tags:   result.NewTags("android-14", "arm"),
			Status: result.Failure,
		},
	}

	err = content.addExpectationsToMutableChunk(&resultList)
	assert.NoErrorf(t, err, "Failed to add expectations: %v", err)

	expectedContent := `# BEGIN TAG HEADER
# OS
# tags: [ android linux win10 ]
# END TAG HEADER

# ##ROLLER_AUTOGENERATED_FAILURES##
crbug.com/0000 [ android-14 arm ] a:b,c:d: [ Failure ]
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]
crbug.com/0000 [ android-14 arm ] z:b,c:d,e:f=g [ Failure ]
`

	assert.Equal(t, expectedContent, content.String())
}

// Tests that a lack of a mutable chunk causes a new mutable chunk to be added
// which has the new expectations.
func TestAddExpectationsToMutableChunkNoMutableChunk(t *testing.T) {
	fileContent := `
# BEGIN TAG HEADER
# OS
# tags: [ android linux win10 ]
# END TAG HEADER

# Immutable
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", fileContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	resultList := result.List{
		result.Result{
			Query:  query.Parse("a:b,c:d"),
			Tags:   result.NewTags("android-14", "arm"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("z:b,c:d,e:f=g"),
			Tags:   result.NewTags("android-14", "arm"),
			Status: result.Failure,
		},
	}

	err = content.addExpectationsToMutableChunk(&resultList)
	assert.NoErrorf(t, err, "Failed to add expectations: %v", err)

	expectedContent := `# BEGIN TAG HEADER
# OS
# tags: [ android linux win10 ]
# END TAG HEADER

# Immutable
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]

################################################################################
# Autogenerated Failure expectations. Please triage.
# ##ROLLER_AUTOGENERATED_FAILURES##
################################################################################
crbug.com/0000 [ android-14 arm ] a:b,c:d: [ Failure ]
crbug.com/0000 [ android-14 arm ] z:b,c:d,e:f=g [ Failure ]
`

	assert.Equal(t, expectedContent, content.String())
}

/*******************************************************************************
 * AddExpectationsForFailingResults tests
 ******************************************************************************/

// Tests overall behavior with an existing mutable chunk.
func TestAddExpectationsForFailingResultsExistingChunk(t *testing.T) {
	fileContent := `
# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 win11 ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         imagination
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#             intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184 nvidia-0x2783
#         qualcomm qualcomm-0x41333430 qualcomm-0x36333630 qualcomm-0x36334330 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] z:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] invalid_test [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Fully removed, immutable.
[ android ] invalid_test [ Failure ]
[ android ] c:* [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] invalid_test [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", fileContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	knownTests := []query.Query{
		query.Parse("valid_test1"),
		query.Parse("valid_test2"),
		query.Parse("a:b,c:d:"),
		query.Parse("z:b,c:d,e:f=g"),
	}

	resultList := result.List{
		result.Result{
			Query:  query.Parse("a:b,c:d"),
			Tags:   result.NewTags("android", "android-14", "arm", "unknown1"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("z:b,c:d,e:f=g"),
			Tags:   result.NewTags("android", "android-14", "arm", "unknown2"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("valid_test1"),
			Tags:   result.NewTags("linux", "intel", "intel-gen-9", "intel-0x3e9b", "unknown3"),
			Status: result.Failure,
		},
		// This should be merged with the one above after only the most explicit
		// tags remain. Multiple generations mapping to the same actual GPU
		// shouldn't happen in real life, but serves to test merging.
		result.Result{
			Query:  query.Parse("valid_test1"),
			Tags:   result.NewTags("linux", "intel", "intel-gen-12", "intel-0x3e9b", "unknown4"),
			Status: result.Failure,
		},
	}

	err = content.AddExpectationsForFailingResults(resultList, knownTests, false)
	assert.NoErrorf(t, err, "Failed to add expectations: %v", err)

	expectedContent := `# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 win11 ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         imagination
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#             intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184 nvidia-0x2783
#         qualcomm qualcomm-0x41333430 qualcomm-0x36333630 qualcomm-0x36334330 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] z:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
crbug.com/0000 [ android-14 arm ] a:b,c:d: [ Failure ]
crbug.com/0000 [ intel-0x3e9b linux ] valid_test1 [ Failure ]
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]
crbug.com/0000 [ android-14 arm ] z:b,c:d,e:f=g [ Failure ]
`

	assert.Equal(t, expectedContent, content.String())
}

// Tests overall behavior without an existing mutable chunk.
func TestAddExpectationsForFailingResultsWithoutExistingChunk(t *testing.T) {
	fileContent := `
# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 win11 ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         imagination
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#             intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184 nvidia-0x2783
#         qualcomm qualcomm-0x41333430 qualcomm-0x36333630 qualcomm-0x36334330 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] z:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] invalid_test [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Fully removed, immutable.
[ android ] invalid_test [ Failure ]
[ android ] c:* [ Failure ]
`

	content, err := Parse("expectations.txt", fileContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	knownTests := []query.Query{
		query.Parse("valid_test1"),
		query.Parse("valid_test2"),
		query.Parse("a:b,c:d:"),
		query.Parse("z:b,c:d,e:f=g"),
	}

	resultList := result.List{
		result.Result{
			Query:  query.Parse("a:b,c:d"),
			Tags:   result.NewTags("android", "android-14", "arm", "unknown1"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("z:b,c:d,e:f=g"),
			Tags:   result.NewTags("android", "android-14", "arm", "unknown2"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("valid_test1"),
			Tags:   result.NewTags("linux", "intel", "intel-gen-9", "intel-0x3e9b", "unknown3"),
			Status: result.Failure,
		},
		// This should be merged with the one above after only the most explicit
		// tags remain. Multiple generations mapping to the same actual GPU
		// shouldn't happen in real life, but serves to test merging.
		result.Result{
			Query:  query.Parse("valid_test1"),
			Tags:   result.NewTags("linux", "intel", "intel-gen-12", "intel-0x3e9b", "unknown4"),
			Status: result.Failure,
		},
	}

	err = content.AddExpectationsForFailingResults(resultList, knownTests, false)
	assert.NoErrorf(t, err, "Failed to add expectations: %v", err)

	expectedContent := `# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 win11 ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         imagination
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#             intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184 nvidia-0x2783
#         qualcomm qualcomm-0x41333430 qualcomm-0x36333630 qualcomm-0x36334330 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] z:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] valid_test2 [ Failure ]

################################################################################
# Autogenerated Failure expectations. Please triage.
# ##ROLLER_AUTOGENERATED_FAILURES##
################################################################################
crbug.com/0000 [ android-14 arm ] a:b,c:d: [ Failure ]
crbug.com/0000 [ intel-0x3e9b linux ] valid_test1 [ Failure ]
crbug.com/0000 [ android-14 arm ] z:b,c:d,e:f=g [ Failure ]
`

	assert.Equal(t, expectedContent, content.String())
}

// Tests overall behavior with multiple mutable chunks.
func TestAddExpectationsForFailingResultsMultipleExistingChunk(t *testing.T) {
	fileContent := `
# BEGIN TAG HEADER (autogenerated, see validate_tag_consistency.py)
# OS
# tags: [ android android-oreo android-pie android-r android-s android-t
#             android-14
#         chromeos
#         fuchsia
#         linux ubuntu
#         mac highsierra mojave catalina bigsur monterey ventura sonoma sequoia
#         win win8 win10 win11 ]
# GPU
# tags: [ amd amd-0x6613 amd-0x679e amd-0x67ef amd-0x6821 amd-0x7340
#         apple apple-apple-m1 apple-apple-m2
#             apple-angle-metal-renderer:-apple-m1
#             apple-angle-metal-renderer:-apple-m2
#         arm
#         google google-0xffff google-0xc0de
#         imagination
#         intel intel-gen-9 intel-gen-12 intel-0xa2e intel-0xd26 intel-0xa011
#             intel-0x3e92 intel-0x3e9b intel-0x4680 intel-0x5912 intel-0x9bc5
#         nvidia nvidia-0xfe9 nvidia-0x1cb3 nvidia-0x2184 nvidia-0x2783
#         qualcomm qualcomm-0x41333430 qualcomm-0x36333630 qualcomm-0x36334330 ]
# END TAG HEADER

# Partially removed, immutable.
[ linux ] valid_test1 [ Failure ]
[ linux ] invalid_test [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Fully removed, immutable.
[ android ] invalid_test [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] invalid_test [ Failure ]
[ win10 ] valid_test2 [ Failure ]

# ##ROLLER_AUTOGENERATED_FAILURES##
[ android ] valid_test1 [ Failure ]
[ android ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", fileContent)
	assert.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	knownTests := []query.Query{
		query.Parse("valid_test1"),
		query.Parse("valid_test2"),
		query.Parse("new_test"),
		query.Parse("z_new_test"),
	}

	resultList := result.List{
		result.Result{
			Query:  query.Parse("new_test"),
			Tags:   result.NewTags("android", "android-14", "arm", "unknown1"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("z_new_test"),
			Tags:   result.NewTags("android", "android-14", "arm", "unknown2"),
			Status: result.Failure,
		},
		result.Result{
			Query:  query.Parse("valid_test1"),
			Tags:   result.NewTags("linux", "intel", "intel-gen-9", "intel-0x3e9b", "unknown3"),
			Status: result.Failure,
		},
		// This should be merged with the one above after only the most explicit
		// tags remain. Multiple generations mapping to the same actual GPU
		// shouldn't happen in real life, but serves to test merging.
		result.Result{
			Query:  query.Parse("valid_test1"),
			Tags:   result.NewTags("linux", "intel", "intel-gen-12", "intel-0x3e9b", "unknown4"),
			Status: result.Failure,
		},
	}

	err = content.AddExpectationsForFailingResults(resultList, knownTests, false)
	assert.EqualError(t, err, "Expected 1 mutable chunk, found 2")
}
