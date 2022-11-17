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

package roll

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/git"
	"github.com/google/go-cmp/cmp"
)

func MustParseHash(s string) git.Hash {
	hash, err := git.ParseHash("d5e605a556408eaeeda64fb9d33c3f596fd90b70")
	if err != nil {
		panic(err)
	}
	return hash
}

func TestRollCommitMessage(t *testing.T) {
	r := roller{
		cfg: common.Config{
			Builders: map[string]buildbucket.Builder{
				"Win":   {Project: "chromium", Bucket: "try", Builder: "win-dawn-rel"},
				"Mac":   {Project: "dawn", Bucket: "try", Builder: "mac-dbg"},
				"Linux": {Project: "chromium", Bucket: "try", Builder: "linux-dawn-rel"},
			},
		},
	}
	msg := r.rollCommitMessage(
		"d5e605a556408eaeeda64fb9d33c3f596fd90b70",
		"29275672eefe76986bd4baa7c29ed17b66616b1b",
		[]git.CommitInfo{
			{
				Hash:    MustParseHash("d5e605a556408eaeeda64fb9d33c3f596fd90b70"),
				Subject: "Added thing A",
			},
			{
				Hash:    MustParseHash("29275672eefe76986bd4baa7c29ed17b66616b1b"),
				Subject: "Tweaked thing B",
			},
		},
		"I4aa059c6c183e622975b74dbdfdfe0b12341ae15",
	)
	expect := `Roll third_party/webgpu-cts/ d5e605a55..29275672e (2 commits)

Regenerated:
 - expectations.txt
 - ts_sources.txt
 - test_list.txt
 - cache_list.txt
 - resource_files.txt
 - webtest .html files


https://chromium.googlesource.com/external/github.com/gpuweb/cts/+log/d5e605a55640..29275672eefe
 - d5e605 Added thing A
 - d5e605 Tweaked thing B

Created with './tools/run cts roll'

Cq-Include-Trybots: luci.chromium.try:linux-dawn-rel,win-dawn-rel;luci.dawn.try:mac-dbg
Include-Ci-Only-Tests: true
Change-Id: I4aa059c6c183e622975b74dbdfdfe0b12341ae15
`
	if diff := cmp.Diff(msg, expect); diff != "" {
		t.Errorf("rollCommitMessage: %v", diff)
	}
}
