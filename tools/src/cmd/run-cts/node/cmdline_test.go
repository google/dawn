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
	"context"
	"os"
	"path/filepath"
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"github.com/stretchr/testify/require"
)

// newTestRunner returns a cmdlineRunner that runs cases with the fake runner
// in a new fake CTS directory, and that directory.
func newTestRunner(t *testing.T, cases ...string) (*cmdlineRunner, string) {
	t.Helper()
	t.Setenv(fakeRunnerEnv, "1")
	exe, err := os.Executable()
	require.NoError(t, err)
	ctsDir := newFakeCTS(t, cases)
	testCases := make([]common.TestCase, len(cases))
	for i, testCase := range cases {
		testCases[i] = common.TestCase(testCase)
	}
	return &cmdlineRunner{
		target:           localTarget{runner: exe},
		args:             []string{"--cts-dir", ctsDir},
		queryStdin:       true,
		cases:            testCases,
		numWorkers:       1,
		batchSize:        defaultBatchSize,
		caseTimeout:      time.Minute,
		startupAllowance: time.Minute,
	}, ctsDir
}

// runCmdline runs runner and returns its results, checking that every test case has
// exactly one result whose Index matches its position.
func runCmdline(t *testing.T, runner *cmdlineRunner) map[common.TestCase]common.Result {
	t.Helper()
	stream := make(chan common.Result, len(runner.cases)+1)
	go func() {
		runner.run(context.Background(), stream)
		close(stream)
	}()
	results := map[common.TestCase]common.Result{}
	for res := range stream {
		_, duplicate := results[res.TestCase]
		require.False(t, duplicate, "two results for %v", res.TestCase)
		require.Equal(t, runner.cases[res.Index], res.TestCase)
		results[res.TestCase] = res
	}
	require.Len(t, results, len(runner.cases))
	return results
}

// The test case in flight when the runner exits early is a failure with the
// output since the last result, and the rest of the batch is reported as not
// run.
func TestRunEarlyExitInBatch(t *testing.T) {
	runner, _ := newTestRunner(t, "suite:a:", "suite:crash:", "suite:c:", "suite:d:")
	results := runCmdline(t, runner)
	require.Equal(t, common.Pass, results["suite:a:"].Status)
	crashed := results["suite:crash:"]
	require.Equal(t, common.Fail, crashed.Status)
	require.Contains(t, crashed.Message, "[ERROR] crashing in suite:crash:\n")
	require.Contains(t, crashed.Message, "the runner process ended before reporting a result")
	for _, testCase := range []common.TestCase{"suite:c:", "suite:d:"} {
		require.Equal(t, common.Fail, results[testCase].Status)
		require.Equal(t, "not run: the runner process ended while running suite:crash:\n", results[testCase].Message)
	}
}

// A runner that exits with status 0 before running every test case is not a
// pass.
func TestRunStall(t *testing.T) {
	runner, _ := newTestRunner(t, "suite:a:", "suite:stall:", "suite:c:")
	results := runCmdline(t, runner)
	require.Equal(t, common.Pass, results["suite:a:"].Status)
	stall := results["suite:stall:"]
	require.Equal(t, common.Fail, stall.Status)
	require.Contains(t, stall.Message, "exit status 0")
	require.Contains(t, stall.Message, "a promise that never settles")
	require.Equal(t, common.Fail, results["suite:c:"].Status)
}

// A test case that reports no result within the case timeout is killed and
// reported as a timeout.
func TestTimeout(t *testing.T) {
	runner, _ := newTestRunner(t, "suite:a:", "suite:hang:", "suite:c:")
	runner.caseTimeout = 500 * time.Millisecond
	start := time.Now()
	results := runCmdline(t, runner)
	require.Less(t, time.Since(start), 30*time.Second)
	require.Equal(t, common.Pass, results["suite:a:"].Status)
	require.Equal(t, common.Timeout, results["suite:hang:"].Status)
	require.Contains(t, results["suite:hang:"].Message, "no result after")
	require.Equal(t, common.Fail, results["suite:c:"].Status)
}

// A process that does not exit after its last test case is killed without
// blaming any test case.
func TestHangAtExit(t *testing.T) {
	runner, _ := newTestRunner(t, "suite:a:", "suite:noexit:")
	runner.caseTimeout = 300 * time.Millisecond
	results := runCmdline(t, runner)
	require.Equal(t, common.Pass, results["suite:a:"].Status)
	require.Equal(t, common.Pass, results["suite:noexit:"].Status)
}

func TestRunMissingRunner(t *testing.T) {
	runner, _ := newTestRunner(t, "suite:a:", "suite:b:")
	runner.target = localTarget{runner: filepath.Join(t.TempDir(), "missing")}
	results := runCmdline(t, runner)
	for _, res := range results {
		require.Equal(t, common.Fail, res.Status)
		require.Contains(t, res.Message, "not run: could not start the runner")
	}
}
