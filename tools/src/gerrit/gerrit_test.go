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

package gerrit_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"github.com/google/go-cmp/cmp"
)

func TestPatchsetRefsChanges(t *testing.T) {
	type Test struct {
		in     gerrit.Patchset
		expect string
	}
	for _, test := range []Test{
		{
			in: gerrit.Patchset{
				Change:   123456,
				Patchset: 42,
			},
			expect: `refs/changes/56/123456/42`,
		},
		{
			in: gerrit.Patchset{
				Change:   1234,
				Patchset: 42,
			},
			expect: `refs/changes/34/1234/42`,
		},
		{
			in: gerrit.Patchset{
				Change:   12,
				Patchset: 42,
			},
			expect: `refs/changes/12/12/42`,
		},
		{
			in: gerrit.Patchset{
				Change:   1,
				Patchset: 42,
			},
			expect: `refs/changes/01/1/42`,
		},
	} {
		got := test.in.RefsChanges()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("%v.RefsChanges() was not as expected:\n%v", test.in, diff)
		}
	}
}
