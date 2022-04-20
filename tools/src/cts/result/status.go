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

package result

import "dawn.googlesource.com/dawn/tools/src/container"

// Status is an enumerator of test results
type Status string

// Enumerator values for Status
const (
	Abort          = Status("Abort")
	Crash          = Status("Crash")
	Failure        = Status("Failure")
	Pass           = Status("Pass")
	RetryOnFailure = Status("RetryOnFailure")
	Skip           = Status("Skip")
	Slow           = Status("Slow")
	Unknown        = Status("Unknown")
)

// CommonStatus is a function that can be used by StatusTree.Reduce() to reduce
// tree nodes with the same status
func CommonStatus(statuses []Status) *Status {
	if set := container.NewSet(statuses...); len(set) == 1 {
		return &statuses[0]
	}
	return nil
}
