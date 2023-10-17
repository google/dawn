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
