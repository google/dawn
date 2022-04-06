// Copyright 2021 The Tint Authors.
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
	"os"
	"path/filepath"
	"strings"
	"testing"

	"dawn.googlesource.com/tint/tools/src/fileutils"
)

func TestGoSourcePath(t *testing.T) {
	p := fileutils.GoSourcePath()
	if !strings.HasSuffix(p, "fileutils/fileutils_test.go") {
		t.Errorf("GoSourcePath() returned %v", p)
	}
}

func TestProjectRoot(t *testing.T) {
	p := filepath.Join(fileutils.ProjectRoot(), "tint_overrides_with_defaults.gni")
	if _, err := os.Stat(p); os.IsNotExist(err) {
		t.Errorf("ProjectRoot() returned %v", p)
	}
}
