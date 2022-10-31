// Copyright 2022 The Tint Authors.
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

package fileutils_test

import (
	"path/filepath"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"github.com/google/go-cmp/cmp"
)

func TestThisLine(t *testing.T) {
	td := fileutils.ThisLine()
	if !strings.HasSuffix(td, "paths_test.go:27") {
		t.Errorf("TestThisLine() returned %v", td)
	}
}

func TestThisDir(t *testing.T) {
	td := fileutils.ThisDir()
	if !strings.HasSuffix(td, "utils") {
		t.Errorf("ThisDir() returned %v", td)
	}
}

func TestDawnRoot(t *testing.T) {
	dr := fileutils.DawnRoot()
	rel, err := filepath.Rel(dr, fileutils.ThisDir())
	if err != nil {
		t.Fatalf("%v", err)
	}
	got := filepath.ToSlash(rel)
	expect := `tools/src/fileutils`
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("DawnRoot() returned %v.\n%v", dr, diff)
	}
}
