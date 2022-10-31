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

	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"github.com/google/go-cmp/cmp"
)

func TestMinimalVariantTags(t *testing.T) {
	type Test struct {
		location string
		results  result.List
		expect   []result.Variant
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			results:  result.List{},
			expect:   []result.Variant{},
		}, { ///////////////////////////////////////////////////////////////////
			// Single variant, that can be entirely optimized away
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b1", "c2"), Status: result.Pass},
			},
			expect: []result.Variant{T()},
		}, { ///////////////////////////////////////////////////////////////////
			// Multiple variants on the same query.
			// Can also be entirely optimized away.
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b1", "c2"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a1", "b2", "c0"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a2", "b1", "c0"), Status: result.Pass},
			},
			expect: []result.Variant{T()},
		}, { ///////////////////////////////////////////////////////////////////
			// Two variants where the 1st and 2nd tag-sets are redundant.
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b0", "c0"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a1", "b1", "c1"), Status: result.Failure},
			},
			expect: []result.Variant{T("c0"), T("c1")},
		}, { ///////////////////////////////////////////////////////////////////
			// Two variants where the 1st and 3rd tag-sets are redundant.
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b0", "c0"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a1", "b1", "c1"), Status: result.Failure},
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b0", "c1"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a1", "b1", "c0"), Status: result.Failure},
			},
			expect: []result.Variant{T("b0"), T("b1")},
		}, { ///////////////////////////////////////////////////////////////////
			// Two variants where the 2nd and 3rd tag-sets are redundant.
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b0", "c0"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a1", "b1", "c1"), Status: result.Failure},
				{Query: Q("a:b,c:d,*"), Tags: T("a0", "b1", "c1"), Status: result.Pass},
				{Query: Q("a:b,c:d,*"), Tags: T("a1", "b0", "c0"), Status: result.Failure},
			},
			expect: []result.Variant{T("a0"), T("a1")},
		}, { ///////////////////////////////////////////////////////////////////
			// Check that variants aren't optimized to expand the set of results
			// they target, even if results are uniform
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d0,*"), Tags: T("a0", "b0", "c0"), Status: result.Pass},
				{Query: Q("a:b,c:d1,*"), Tags: T("a1", "b1", "c1"), Status: result.Pass},
			},
			expect: []result.Variant{T("c0"), T("c1")},
		}, { ///////////////////////////////////////////////////////////////////
			// Exercise the optimizations to skip checks on tag removals that
			// aren't found in all variants
			location: fileutils.ThisLine(),
			results: result.List{
				{Query: Q("a:b,c:d0,*"), Tags: T("a0"), Status: result.Pass},
				{Query: Q("a:b,c:d1,*"), Tags: T("b0"), Status: result.Pass},
				{Query: Q("a:b,c:d2,*"), Tags: T("c0"), Status: result.Pass},
			},
			expect: []result.Variant{T("a0"), T("b0"), T("c0")},
		},
	} {
		preReduce := fmt.Sprint(test.results)
		got := test.results.MinimalVariantTags([]result.Tags{
			T("a0", "a1", "a2"),
			T("b0", "b1", "b2"),
			T("c0", "c1", "c2"),
		})
		postReduce := fmt.Sprint(test.results)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("%v MinimalVariantTags() diff:\n%v", test.location, diff)
		}
		if diff := cmp.Diff(preReduce, postReduce); diff != "" {
			t.Errorf("%v MinimalVariantTags() modified original list:\n%v", test.location, diff)
		}
	}
}
