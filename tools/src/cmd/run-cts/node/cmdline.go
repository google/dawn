// Copyright 2023 The Dawn & Tint Authors
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
	"errors"
	"fmt"
	"os/exec"
	"path/filepath"
	"sync"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// TODO(crbug.com/416755658): Add unittest coverage when there is a way to fake
// the node process.
// runTestCasesWithCmdline() calls the CTS command-line test runner to run each
// test case in a separate process. This reduces possibility of state leakage
// between tests.
// Up to c.flags.numRunners tests will be run concurrently.
func (c *cmd) runTestCasesWithCmdline(
	ctx context.Context, testCases []common.TestCase, results chan<- common.Result, fsReaderWriter oswrapper.FilesystemReaderWriter) {
	// Create a chan of test indices.
	// This will be read by the test runner goroutines.
	testCaseIndices := make(chan int, 256)
	go func() {
		for i := range testCases {
			testCaseIndices <- i
		}
		close(testCaseIndices)
	}()

	// Spin up the test runner goroutines
	wg := &sync.WaitGroup{}
	for i := 0; i < c.flags.NumRunners; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for idx := range testCaseIndices {
				res := c.runTestCaseWithCmdline(ctx, testCases[idx], fsReaderWriter)
				res.Index = idx
				results <- res
				if err := ctx.Err(); err != nil {
					return
				}
			}
		}()
	}
	wg.Wait()
}

// runTestCaseWithCmdline runs a single CTS test case in a command-line runner
// process and returns the result.
func (c *cmd) runTestCaseWithCmdline(ctx context.Context, testCase common.TestCase, fsReaderWriter oswrapper.FilesystemReaderWriter) common.Result {
	ctx, cancel := context.WithTimeout(ctx, common.TestCaseTimeout)
	defer cancel()

	executable, args := c.cmdlineTarget(testCase)
	cmd := exec.CommandContext(ctx, executable, args...)
	cmd.Dir = c.flags.CTS
	if c.flags.Verbose {
		PrintCommand(cmd, c.flags.skipVSCodeInfo)
	}

	proc, err := startWithPipe(cmd)
	if err != nil {
		return common.Result{
			TestCase: testCase,
			Status:   common.Fail,
			Error:    err,
		}
	}

	lines := make(chan string, 64)
	go readLines(proc.out, lines)

	// The runner prints whatever the test case writes, then one result line,
	// `[<status>] <case> (<ms>ms). Log:`, then the test case's log lines, each
	// indented. With --coverage, a `Code-coverage: [[<path>]]` line comes last.
	var output []string       // lines printed before the result line
	var result *common.Result // set by the result line
	for line := range lines {
		// The coverage data is attached to the result, not added to its message.
		if c.coverage != nil {
			if path, ok := parseCoverageLine(line); ok {
				c.importCoverage(path, result, fsReaderWriter)
				continue
			}
		}

		// The indented lines after the result line are its log lines. Other lines
		// after it, such as warnings Dawn prints when the process exits, are
		// dropped. They are unrelated to the test case. If they were, they would
		// have been printed before the result line.
		if result != nil {
			if isLogLine(line) {
				result.Message += line + "\n"
			}
			continue
		}

		// Every line before the result line is output of the test case.
		word, _, duration, ok := parseResultLine(line)
		if !ok {
			output = append(output, line)
			continue
		}

		// The result's message starts with the output read so far, and the log
		// lines that follow are appended to it.
		status, known := toStatus(word)
		result = &common.Result{TestCase: testCase, Status: status, Duration: duration, Message: joinLines(output)}
		if !known {
			result.Error = fmt.Errorf("unknown status '%v'", word)
		}
	}
	exit := proc.wait()
	if result != nil {
		return *result
	}
	if errors.Is(ctx.Err(), context.DeadlineExceeded) {
		return common.Result{
			TestCase: testCase,
			Status:   common.Timeout,
			Message:  joinLines(output),
		}
	}
	msg := joinLines(output) +
		"the runner process ended before reporting a result for this test case: " + exit.description + "\n"
	if exit.success {
		msg += "an early exit with status 0 usually means that a test awaited a promise that never settles\n"
	}
	return common.Result{
		TestCase: testCase,
		Status:   common.Fail,
		Message:  msg,
	}
}

// importCoverage imports coverage data from path into res.
func (c *cmd) importCoverage(path string, res *common.Result, fsReaderWriter oswrapper.FilesystemReaderWriter) {
	if res == nil {
		return
	}
	coverage, err := c.coverage.Env.Import(path, fsReaderWriter)
	fsReaderWriter.Remove(path)
	if err != nil {
		res.Error = fmt.Errorf("could not import coverage data from '%v': %v", path, err)
		res.Status = common.Fail
		return
	}
	res.Coverage = coverage
}

// cmdlineTarget returns the executable and arguments to run testCase in a
// command-line runner process.
func (c *cmd) cmdlineTarget(testCase common.TestCase) (string, []string) {
	args := append(c.runnerArgs(c.flags.CTS), "--verbose", "--quiet")
	if c.standalone {
		return c.flags.runner, append(args, "-q", string(testCase))
	}
	return c.flags.Node, append(args, string(testCase))
}

// runnerArgs returns the command-line runner arguments shared by every process.
func (c *cmd) runnerArgs(ctsDir string) []string {
	var args []string
	var dawnFlag string
	if c.standalone {
		args = []string{"--cts-dir", ctsDir}
		dawnFlag = "--dawn-flag"
	} else {
		args = []string{
			"-e", "require('./out-node/common/runtime/cmdline.js');",
			"--",
			// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
			// and slices away the first two arguments. When running with '-e', args
			// start at 1, so just inject a placeholder argument.
			"placeholder-arg",
			"--gpu-provider", filepath.Join(c.flags.bin, "cts.cjs"),
		}
		dawnFlag = "--gpu-provider-flag"
	}
	if c.flags.Verbose {
		args = append(args, dawnFlag, "verbose=1")
	}
	if c.coverage != nil {
		args = append(args, "--coverage")
	}
	// --colors is a cmdline.js option; the standalone runner rejects it.
	if c.flags.Colors && !c.standalone {
		args = append(args, "--colors")
	}
	if c.flags.unrollConstEvalLoops {
		args = append(args, "--unroll-const-eval-loops")
	}
	if c.flags.compatibilityMode {
		args = append(args, "--compat")
	}
	if c.flags.enforceDefaultLimits {
		args = append(args, "--enforce-default-limits")
	}
	if c.flags.blockAllFeatures {
		args = append(args, "--block-all-features")
	}
	if c.flags.debugCTS {
		args = append(args, "--debug")
	}
	for _, f := range c.flags.dawn {
		args = append(args, dawnFlag, f)
	}
	return args
}
