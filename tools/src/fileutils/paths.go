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

package fileutils

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

// ThisLine returns the filepath and line number of the calling function
func ThisLine() string {
	_, file, line, ok := runtime.Caller(1)
	if !ok {
		return ""
	}
	return fmt.Sprintf("%v:%v", file, line)
}

// ThisDir returns the directory of the caller function
func ThisDir() string {
	_, file, _, ok := runtime.Caller(1)
	if !ok {
		return ""
	}
	return filepath.Dir(file)
}

// DawnRoot returns the path to the dawn project's root directory or empty
// string if not found.
func DawnRoot() string {
	return pathOfFileInParentDirs(ThisDir(), "DEPS")
}

// pathOfFileInParentDirs looks for file with `name` in paths starting from
// `path`, and up into parent directories, returning the clean path in which the
// file is found, or empty string if not found.
func pathOfFileInParentDirs(path string, name string) string {
	sep := string(filepath.Separator)
	path, _ = filepath.Abs(path)
	numDirs := strings.Count(path, sep) + 1
	for i := 0; i < numDirs; i++ {
		test := filepath.Join(path, name)
		if _, err := os.Stat(test); err == nil {
			return filepath.Clean(path)
		}

		path = path + sep + ".."
	}
	return ""
}

// ExpandHome returns the string with all occurrences of '~' replaced with the
// user's home directory. The the user's home directory cannot be found, then
// the input string is returned.
func ExpandHome(path string) string {
	if strings.ContainsRune(path, '~') {
		if home, err := os.UserHomeDir(); err == nil {
			return strings.ReplaceAll(path, "~", home)
		}
	}
	return path
}
