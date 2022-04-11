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

package result_test

import (
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"github.com/google/go-cmp/cmp"
)

var Q = query.Parse

func T(tags ...string) result.Tags {
	return result.NewTags(tags...)
}

func TestStringAndParse(t *testing.T) {
	type Test struct {
		result result.Result
		expect string
	}
	for _, test := range []Test{
		{
			result.Result{
				Query:  Q(`a`),
				Status: result.Failure,
			},
			`a Failure`,
		}, {
			result.Result{
				Query:  Q(`a:b,c,*`),
				Tags:   T("x"),
				Status: result.Pass,
			},
			`a:b,c,* x Pass`,
		},
		{
			result.Result{
				Query:  Q(`a:b,c:d,*`),
				Tags:   T("zzz", "x", "yy"),
				Status: result.Failure,
			},
			`a:b,c:d,* x,yy,zzz Failure`,
		},
	} {
		if diff := cmp.Diff(test.result.String(), test.expect); diff != "" {
			t.Errorf("'%v'.String() was not as expected:\n%v", test.result, diff)
			continue
		}
		parsed, err := result.Parse(test.expect)
		if err != nil {
			t.Errorf("Parse('%v') returned %v", test.expect, err)
			continue
		}
		if diff := cmp.Diff(parsed, test.result); diff != "" {
			t.Errorf("Parse('%v') was not as expected:\n%v", test.expect, diff)
		}
	}
}

func TestParseError(t *testing.T) {
	for _, test := range []string{
		``,
		`a`,
		`a b c d`,
	} {
		_, err := result.Parse(test)
		expect := fmt.Sprintf(`unable to parse result '%v'`, test)
		if err == nil || err.Error() != expect {
			t.Errorf("Parse('%v') returned '%v'", test, err)
			continue
		}
	}
}

func TestUniqueTags(t *testing.T) {
	type Test struct {
		results result.List
		expect  []result.Tags
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
			expect: []result.Tags{
				result.NewTags(),
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
			expect: []result.Tags{
				result.NewTags("x"),
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			expect: []result.Tags{
				result.NewTags("x", "y"),
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			expect: []result.Tags{
				result.NewTags("x", "y"),
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
			},
			expect: []result.Tags{
				result.NewTags("x", "y"),
				result.NewTags("y", "z"),
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("z", "x"),
				},
				result.Result{
					Query:  Q(`d`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
			},
			expect: []result.Tags{
				result.NewTags("x", "y"),
				result.NewTags("x", "z"),
				result.NewTags("y", "z"),
			},
		},
	} {
		got := test.results.UniqueTags()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nUniqueTags() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestTransformTags(t *testing.T) {
	type Test struct {
		results   result.List
		transform func(result.Tags) result.Tags
		expect    result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
			transform: func(t result.Tags) result.Tags { return t },
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
			transform: func(got result.Tags) result.Tags {
				expect := result.NewTags("x")
				if diff := cmp.Diff(got, expect); diff != "" {
					t.Errorf("transform function's parameter was not as expected:\n%v", diff)
				}
				return got
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("z", "x"),
				},
			},
			transform: func(l result.Tags) result.Tags {
				l = l.Clone()
				if l.Contains("x") {
					l.Remove("x")
					l.Add("X")
				}
				return l
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("X", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("z", "X"),
				},
			},
		},
	} {
		got := test.results.TransformTags(test.transform)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nTransformTags() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestReplaceDuplicates(t *testing.T) {
	type Test struct {
		results result.List
		fn      func(result.List) result.Status
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			fn: func(l result.List) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			fn: func(l result.List) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Abort},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
			fn: func(l result.List) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Skip},
			},
			fn: func(got result.List) result.Status {
				expect := result.List{
					result.Result{Query: Q(`a`), Status: result.Pass},
					result.Result{Query: Q(`a`), Status: result.Skip},
				}
				if diff := cmp.Diff(got, expect); diff != "" {
					t.Errorf("function's parameter was not as expected:\n%v", diff)
				}
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Abort},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
	} {
		got := test.results.ReplaceDuplicates(test.fn)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nReplaceDuplicates() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestSort(t *testing.T) {
	type Test struct {
		results result.List
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`b`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Skip},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Skip,
					Tags:   result.NewTags(),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Skip,
					Tags:   result.NewTags(),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`c`),
					Status: result.RetryOnFailure,
					Tags:   result.NewTags("z"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Slow,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`aa`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Abort,
					Tags:   result.NewTags("z"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`aa`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Slow,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Abort,
					Tags:   result.NewTags("z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.RetryOnFailure,
					Tags:   result.NewTags("z"),
				},
			},
		},
	} {
		got := make(result.List, len(test.results))
		copy(got, test.results)
		got.Sort()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nSort() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestFilter(t *testing.T) {
	type Test struct {
		results result.List
		f       func(result.Result) bool
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			f: func(result.Result) bool { return true },
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
			f: func(r result.Result) bool { return r.Query == Q("b") },
			expect: result.List{
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Skip},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
			f: func(r result.Result) bool { return r.Status == result.Pass },
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
		},
	} {
		got := test.results.Filter(test.f)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nFilter() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestFilterByStatus(t *testing.T) {
	type Test struct {
		results result.List
		status  result.Status
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
			status: result.Pass,
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
			status: result.Failure,
			expect: result.List{
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
			status: result.RetryOnFailure,
			expect: result.List{},
		},
	} {
		got := test.results.FilterByStatus(test.status)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nFilterByStatus(%v) was not as expected:\n%v", test.results, test.status, diff)
		}
	}
}

func TestFilterByTags(t *testing.T) {
	type Test struct {
		results result.List
		tags    result.Tags
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Failure,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			tags: result.NewTags("x", "y"),
			expect: result.List{
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Failure,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			tags: result.NewTags("x"),
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Failure,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			tags:   result.NewTags("q"),
			expect: result.List{},
		},
	} {
		got := test.results.FilterByTags(test.tags)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nFilterByTags(%v) was not as expected:\n%v", test.results, test.tags, diff)
		}
	}
}

func TestStatuses(t *testing.T) {
	type Test struct {
		results result.List
		expect  container.Set[result.Status]
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{},
			expect:  container.NewSet[result.Status](),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
			},
			expect: container.NewSet(result.Pass),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
				},
			},
			expect: container.NewSet(result.Pass),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
				},
			},
			expect: container.NewSet(result.Pass, result.Skip),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Failure,
				},
			},
			expect: container.NewSet(result.Pass, result.Skip, result.Failure),
		},
	} {
		got := test.results.Statuses()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nStatuses() was not as expected:\n%v", test.results, diff)
		}
	}
}
