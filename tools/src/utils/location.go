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

package utils

import (
	"fmt"
	"path/filepath"
	"runtime"
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
