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

package utils_test

import (
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/utils"
)

func TestThisLine(t *testing.T) {
	td := utils.ThisLine()
	if !strings.HasSuffix(td, "location_test.go:25") {
		t.Errorf("TestThisLine() returned %v", td)
	}
}

func TestThisDir(t *testing.T) {
	td := utils.ThisDir()
	if !strings.HasSuffix(td, "utils") {
		t.Errorf("ThisDir() returned %v", td)
	}
}
