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

import (
	"strings"
)

// TargetKind is an enumerator of target kinds
type TargetKind string

const (
	// A library target, used for production code.
	targetLib TargetKind = "lib"
	// A library target, used for test binaries.
	targetTest TargetKind = "test"
	// A library target, used for benchmark binaries.
	targetBench TargetKind = "bench"
	// A library target, used for fuzzer binaries.
	targetFuzz TargetKind = "fuzz"
	// An executable target.
	targetCmd TargetKind = "cmd"
	// A test executable target.
	targetTestCmd TargetKind = "test_cmd"
	// A benchmark executable target.
	targetBenchCmd TargetKind = "bench_cmd"
	// A fuzzer executable target.
	targetFuzzCmd TargetKind = "fuzz_cmd"
	// An invalid target.
	targetInvalid TargetKind = "<invalid>"
)

// IsLib returns true if the TargetKind is 'lib'
func (k TargetKind) IsLib() bool { return k == targetLib }

// IsTest returns true if the TargetKind is 'test'
func (k TargetKind) IsTest() bool { return k == targetTest }

// IsBench returns true if the TargetKind is 'bench'
func (k TargetKind) IsBench() bool { return k == targetBench }

// IsBench returns true if the TargetKind is 'fuzz'
func (k TargetKind) IsFuzz() bool { return k == targetFuzz }

// IsCmd returns true if the TargetKind is 'cmd'
func (k TargetKind) IsCmd() bool { return k == targetCmd }

// IsTestCmd returns true if the TargetKind is 'test_cmd'
func (k TargetKind) IsTestCmd() bool { return k == targetTestCmd }

// IsBenchCmd returns true if the TargetKind is 'bench_cmd'
func (k TargetKind) IsBenchCmd() bool { return k == targetBenchCmd }

// IsFuzzCmd returns true if the TargetKind is 'fuzz_cmd'
func (k TargetKind) IsFuzzCmd() bool { return k == targetFuzzCmd }

// IsTestOrTestCmd returns true if the TargetKind is 'test' or 'test_cmd'
func (k TargetKind) IsTestOrTestCmd() bool { return k.IsTest() || k.IsTestCmd() }

// IsBenchOrBenchCmd returns true if the TargetKind is 'bench' or 'bench_cmd'
func (k TargetKind) IsBenchOrBenchCmd() bool { return k.IsBench() || k.IsBenchCmd() }

// All the target kinds
var AllTargetKinds = []TargetKind{
	targetLib,
	targetTest,
	targetBench,
	targetFuzz,
	targetCmd,
	targetTestCmd,
	targetBenchCmd,
	targetFuzzCmd,
}

// targetKindFromFilename returns the target kind my pattern matching the filename
func targetKindFromFilename(filename string) TargetKind {
	noExt, ext := filename, ""
	if i := strings.LastIndex(filename, "."); i >= 0 {
		noExt = filename[:i]
		ext = filename[i+1:]
	}

	if ext != "cc" && ext != "mm" && ext != "h" {
		return targetInvalid
	}

	switch {
	case filename == "main_test.cc":
		return targetTestCmd
	case filename == "main_bench.cc":
		return targetBenchCmd
	case filename == "main_fuzz.cc":
		return targetFuzzCmd
	case strings.HasSuffix(noExt, "_test"):
		return targetTest
	case noExt == "bench" || strings.HasSuffix(noExt, "_bench"):
		return targetBench
	case noExt == "fuzz" || strings.HasSuffix(noExt, "_fuzz"):
		return targetFuzz
	case noExt == "main" || strings.HasSuffix(noExt, "_main"):
		return targetCmd
	default:
		return targetLib
	}
}

// isValidDependency returns true iff its valid for a target of kind 'from' to
// depend on a target with kind 'to'.
func isValidDependency(from, to TargetKind) bool {
	switch from {
	case targetLib, targetCmd:
		return to == targetLib
	case targetTest, targetTestCmd:
		return to == targetLib || to == targetTest
	case targetBench, targetBenchCmd:
		return to == targetLib || to == targetBench
	case targetFuzz, targetFuzzCmd:
		return to == targetLib || to == targetFuzz
	default:
		return false
	}
}
