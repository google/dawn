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
	"bytes"
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/term"
	"github.com/stretchr/testify/require"
)

// listHeading returns how the heading of a list is printed.
func listHeading(status Status, heading string) string {
	return fmt.Sprint("\n", term.Bold, statusColor[status], heading, ":", term.Reset, "\n")
}

// The unexpected results are listed by status, in the order of their indices
// however they arrive, and a result that is no failure is listed as a pass.
// A blank line follows the lists, and no results print nothing.
func TestResultLists(t *testing.T) {
	stdout := &bytes.Buffer{}
	lists := resultLists{withExpectationsFile: true}
	lists.print(stdout)
	require.Empty(t, stdout.String())

	for _, result := range []Result{
		{Index: 5, TestCase: "suite:f:", Status: Warn},
		{Index: 4, TestCase: "suite:e:", Status: Timeout},
		{Index: 3, TestCase: "suite:d:", Status: Fail},
		{Index: 2, TestCase: "suite:c:", Status: Pass},
		{Index: 1, TestCase: "suite:b:", Status: Crash},
		{Index: 0, TestCase: "suite:a:", Status: Fail},
	} {
		lists.add(result)
	}
	lists.print(stdout)
	require.Equal(t,
		listHeading(Fail, "Unexpected failures (2)")+"  suite:a:\n  suite:d:\n"+
			listHeading(Crash, "Unexpected crashes (1)")+"  suite:b:\n"+
			listHeading(Timeout, "Unexpected timeouts (1)")+"  suite:e:\n"+
			listHeading(Pass, "Unexpected passes (2)")+"  suite:c:\n  suite:f:\n\n",
		stdout.String())
}

// Without an expectations file the headings do not say "Unexpected".
func TestResultListsWithoutExpectationsFile(t *testing.T) {
	stdout := &bytes.Buffer{}
	lists := resultLists{}
	lists.add(Result{Index: 1, TestCase: "suite:b:", Status: Fail})
	lists.print(stdout)
	require.Equal(t, listHeading(Fail, "Failures (1)")+"  suite:b:\n\n", stdout.String())
}
