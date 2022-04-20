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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"github.com/google/go-cmp/cmp"
)

func TestCommonStatus(t *testing.T) {
	pass := result.Pass

	type Test struct {
		in     []result.Status
		expect *result.Status
	}
	for _, test := range []Test{
		{
			in:     nil,
			expect: nil,
		}, {
			in:     []result.Status{},
			expect: nil,
		}, {
			in:     []result.Status{result.Pass},
			expect: &pass,
		}, {
			in:     []result.Status{result.Pass, result.Pass, result.Pass},
			expect: &pass,
		}, {
			in:     []result.Status{result.Pass, result.Failure, result.Pass},
			expect: nil,
		},
	} {
		got := result.CommonStatus(test.in)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("%v.CommonStatus('%v') was not as expected:\n%v", test.in, test.expect, diff)
		}
	}
}
