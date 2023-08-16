// Copyright 2023 The Tint Authors.
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

package build

import "strings"

// TargetKind is an enumerator of target kinds
type TargetKind string

const (
	// A library target, used for production code.
	targetLib TargetKind = "lib"
	// A library target, used for test binaries.
	targetTest TargetKind = "test"
	// A library target, used for benchmark binaries.
	targetBench TargetKind = "bench"
	// An executable target.
	targetCmd TargetKind = "cmd"
	// A test executable target.
	targetTestCmd TargetKind = "test_cmd"
	// A benchmark executable target.
	targetBenchCmd TargetKind = "bench_cmd"
	// An invalid target.
	targetInvalid TargetKind = "<invalid>"
)

// All the target kinds
var AllTargetKinds = []TargetKind{
	targetLib,
	targetTest,
	targetBench,
	targetCmd,
	targetTestCmd,
	targetBenchCmd,
}

// targetKindFromFilename returns the target kind my pattern matching the filename
func targetKindFromFilename(filename string) TargetKind {
	switch {
	case filename == "main_test.cc":
		return targetTestCmd
	case filename == "main_bench.cc":
		return targetBenchCmd
	case strings.HasSuffix(filename, "_test.cc"), strings.HasSuffix(filename, "_test.h"):
		return targetTest
	case strings.HasSuffix(filename, "_bench.cc"), strings.HasSuffix(filename, "_bench.h"):
		return targetBench
	case filename == "main.cc":
		return targetCmd
	case strings.HasSuffix(filename, ".cc"), strings.HasSuffix(filename, ".h"):
		return targetLib
	}
	return targetInvalid
}
