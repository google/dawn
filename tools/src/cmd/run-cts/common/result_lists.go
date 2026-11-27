// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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

package common

import (
	"fmt"
	"io"
	"slices"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/term"
)

// resultLists are the test cases that are listed after the run: those that
// failed, crashed or timed out unexpectedly and, with an expectations file,
// those that unexpectedly did not.
type resultLists struct {
	withExpectationsFile                bool // the headings say "Unexpected"
	failures, crashes, timeouts, passes []Result
}

// add adds result, which is unexpected, to the list for its status.
func (l *resultLists) add(result Result) {
	switch result.Status {
	case Fail:
		l.failures = append(l.failures, result)
	case Crash:
		l.crashes = append(l.crashes, result)
	case Timeout:
		l.timeouts = append(l.timeouts, result)
	default:
		l.passes = append(l.passes, result)
	}
}

// print prints the lists that are not empty, each in the order of the indices
// of its results, and then a blank line. It prints nothing if all are empty.
func (l *resultLists) print(stdout io.Writer) {
	if len(l.failures)+len(l.crashes)+len(l.timeouts)+len(l.passes) == 0 {
		return
	}
	for _, results := range [][]Result{l.failures, l.crashes, l.timeouts, l.passes} {
		slices.SortFunc(results, compareIndex)
	}
	printTestCases(stdout, statusColor[Fail], l.heading("failures"), l.failures)
	printTestCases(stdout, statusColor[Crash], l.heading("crashes"), l.crashes)
	printTestCases(stdout, statusColor[Timeout], l.heading("timeouts"), l.timeouts)
	printTestCases(stdout, statusColor[Pass], l.heading("passes"), l.passes)
	fmt.Fprintln(stdout)
}

// heading returns the heading for the list of name, such as "failures":
// "Unexpected failures" with an expectations file, and "Failures" without.
func (l *resultLists) heading(name string) string {
	if l.withExpectationsFile {
		return "Unexpected " + name
	}
	return strings.ToUpper(name[:1]) + name[1:]
}

// compareIndex orders the result with the lower index first.
func compareIndex(one, other Result) int {
	return one.Index - other.Index
}

// printTestCases prints heading with the number of results, then the test case
// of each result. It prints nothing for no results.
func printTestCases(stdout io.Writer, color, heading string, results []Result) {
	if len(results) == 0 {
		return
	}
	fmt.Fprintf(stdout, "\n%v%v%v (%v):%v\n", term.Bold, color, heading, len(results), term.Reset)
	for _, result := range results {
		fmt.Fprintf(stdout, "  %v\n", result.TestCase)
	}
}
