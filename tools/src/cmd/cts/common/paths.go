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

package common

import (
	"os"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

const (
	// RelativeExpectationsPath is the dawn-root relative path to the
	// expectations.txt file.
	RelativeExpectationsPath = "webgpu-cts/expectations.txt"

	// RelativeTestListPath is the dawn-root relative path to the
	// test_list.txt file.
	RelativeTestListPath = "third_party/gn/webgpu-cts/test_list.txt"
)

// DefaultExpectationsPath returns the default path to the expectations.txt
// file. Returns an empty string if the file cannot be found.
func DefaultExpectationsPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeExpectationsPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}

// DefaultTestListPath returns the default path to the test_list.txt
// file. Returns an empty string if the file cannot be found.
func DefaultTestListPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeTestListPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}
