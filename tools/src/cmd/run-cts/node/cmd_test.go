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

package node

import (
	"os"
	"testing"

	"github.com/stretchr/testify/require"
)

// newListCmd returns a standalone command that lists queries with the fake
// runner, in a fake CTS directory that holds cases.
func newListCmd(t *testing.T, cases []string, queries ...string) *cmd {
	t.Helper()
	t.Setenv(fakeRunnerEnv, "1")
	exe, err := os.Executable()
	require.NoError(t, err)
	c := &cmd{standalone: true, queries: queries}
	c.flags.runner = exe
	c.flags.CTS = newFakeCTS(t, cases)
	return c
}

// A query that the runner cannot list stops the run with the runner's output.
func TestListFailure(t *testing.T) {
	c := newListCmd(t, []string{"suite:a:"}, "suite:b:*")
	_, err := c.queryTestCases()
	require.ErrorContains(t, err, "Query `suite:b:*` does not match any cases")
}

// Two queries that match the same test case stop the run.
func TestListOverlappingQueries(t *testing.T) {
	c := newListCmd(t, []string{"suite:a:", "suite:b:"}, "suite:*", "suite:b:")
	_, err := c.queryTestCases()
	require.ErrorContains(t, err, "'suite:b:' matches both 'suite:*' and 'suite:b:'")
}
