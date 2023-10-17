// Copyright 2023 The Dawn & Tint Authors.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
