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
	"fmt"
	"path/filepath"
	"slices"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// defaultBatchSize is the default number of test cases per standalone runner
// process.
const defaultBatchSize = 500

// defaultStartupAllowance is the extra time a runner process gets before its
// first result line, to cover starting the process and loading the CTS. The
// value is arbitrary: start-up takes about 30s on a Pixel 10, the slowest
// device tried so far. Adjust it if slower devices time out at start-up.
const defaultStartupAllowance = 2 * time.Minute

// newCmdlineRunner returns a cmdlineRunner that runs testCases with the
// command's flags.
func (c *cmd) newCmdlineRunner(testCases []common.TestCase, fsReaderWriter oswrapper.FilesystemReaderWriter) *cmdlineRunner {
	target, ctsDir := c.target()
	numWorkers := c.flags.NumRunners
	if numWorkers < 1 {
		numWorkers = 1
	}
	batchSize := 1
	if c.standalone {
		batchSize = c.flags.batchSize
	}
	return &cmdlineRunner{
		target:           target,
		args:             c.runnerArgs(ctsDir),
		queryStdin:       c.standalone,
		cases:            testCases,
		numWorkers:       numWorkers,
		batchSize:        batchSize,
		caseTimeout:      c.flags.caseTimeout,
		startupAllowance: c.flags.startupAllowance,
		recover:          c.flags.recover,
		coverage:         c.coverage,
		fsReaderWriter:   fsReaderWriter,
	}
}

// runTestCasesWithCmdline runs testCases in command-line runner processes and
// streams their results to results.
func (c *cmd) runTestCasesWithCmdline(
	ctx context.Context, testCases []common.TestCase, results chan<- common.Result, fsReaderWriter oswrapper.FilesystemReaderWriter) {
	c.newCmdlineRunner(testCases, fsReaderWriter).run(ctx, results)
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

// cmdlineRunner runs test cases in command-line runner processes and turns
// their output into results. Every test case gets exactly one result. When a
// runner process ends, or stops reporting results, before it has run all its
// test cases, the test case it was running is reported as a crash or a timeout
// and a new process runs the test cases after it.
type cmdlineRunner struct {
	target           localTarget
	args             []string          // runner arguments shared by every process
	queryStdin       bool              // pass queries on stdin via --query-file - instead of argv
	cases            []common.TestCase // test cases to run, in the order the runner runs them
	numWorkers       int               // number of runner processes running at once
	batchSize        int               // test cases per batch
	caseTimeout      time.Duration     // longest wait for the next result line
	startupAllowance time.Duration     // extra wait for the first result line of a process
	recover          bool              // start a new process after a crash or a timeout
	coverage         *common.Coverage
	fsReaderWriter   oswrapper.FilesystemReaderWriter

	results chan<- common.Result
	index   map[common.TestCase]int // position of each test case in cases
}

// caseRange is a contiguous range of test cases that a worker runs.
type caseRange struct {
	start, end int
}

// processOutcome describes how a runner process ended.
type processOutcome struct {
	next     int           // index of the first test case without a result
	output   string        // output since the last result line
	exit     exitStatus    // how the process ended
	timedOut bool          // the watchdog killed the process
	waited   time.Duration // how long the watchdog waited before killing it
	startErr error         // the process could not be started
}

// run runs every test case, sending the results to results.
func (r *cmdlineRunner) run(ctx context.Context, results chan<- common.Result) {
	r.results = results
	r.index = make(map[common.TestCase]int, len(r.cases))
	for i, testCase := range r.cases {
		r.index[testCase] = i
	}

	batches := makeBatches(len(r.cases), r.batchSize, r.numWorkers)
	queue := make(chan caseRange, len(batches))
	for _, batch := range batches {
		queue <- batch
	}
	close(queue)

	wg := sync.WaitGroup{}
	for range min(r.numWorkers, len(batches)) {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for batch := range queue {
				r.runBatch(ctx, batch)
			}
		}()
	}
	wg.Wait()
}

// makeBatches splits numCases test cases into contiguous batches of at most
// batchSize test cases, and of fewer when batches of batchSize would leave some
// of the numWorkers workers without one.
func makeBatches(numCases, batchSize, numWorkers int) []caseRange {
	batchSize = max(1, min(batchSize, (numCases+numWorkers-1)/numWorkers))
	batches := []caseRange{}
	for start := 0; start < numCases; start += batchSize {
		batches = append(batches, caseRange{start: start, end: min(start+batchSize, numCases)})
	}
	return batches
}

// runBatch runs the test cases of batch. When a runner process ends, or stops
// reporting results, before it has run them all, the test case it was running
// is reported as a crash or a timeout and a new process runs the rest.
func (r *cmdlineRunner) runBatch(ctx context.Context, batch caseRange) {
	next := batch.start
	for next < batch.end {
		outcome := r.runProcess(ctx, batch, next, r.results)
		next = outcome.next
		if next == batch.end {
			return
		}
		switch {
		case ctx.Err() != nil:
			r.reportNotRun(r.results, next, batch.end, "not run: the run was interrupted\n")
			return
		case outcome.startErr != nil:
			r.reportNotRun(r.results, next, batch.end, fmt.Sprintf("not run: could not start the runner: %v\n", outcome.startErr))
			return
		}
		r.reportUnfinished(next, outcome)
		next++
		if !r.recover {
			r.reportNotRun(r.results, next, batch.end, fmt.Sprintf("not run: the runner process ended while running %v\n", r.cases[next-1]))
			return
		}
	}
}

// runProcess starts a runner process for the test cases of batch from index next
// and watches it until it exits, sending results to results.
func (r *cmdlineRunner) runProcess(ctx context.Context, batch caseRange, next int, results chan<- common.Result) processOutcome {
	args, stdin := r.processInput(batch, next)
	proc, err := r.target.start(args, stdin)
	if err != nil {
		return processOutcome{next: next, startErr: err}
	}
	return r.watch(ctx, proc, next, batch.end, results)
}

// processInput returns the arguments and standard input of a runner process
// that runs the test cases of batch from index next.
func (r *cmdlineRunner) processInput(batch caseRange, next int) ([]string, string) {
	queries := make([]string, 0, batch.end-next)
	for _, testCase := range r.cases[next:batch.end] {
		queries = append(queries, string(testCase))
	}
	args := append(slices.Clone(r.args), "--verbose", "--quiet")
	if r.queryStdin {
		return append(args, "--query-file", "-"), joinLines(queries)
	}
	return append(args, queries...), ""
}

// watch reads the output of proc and sends the result of each test case in
// [next, end) that proc finishes to results. If no result line arrives within
// the case timeout, plus the start-up allowance for the first one, it kills proc.
func (r *cmdlineRunner) watch(ctx context.Context, proc *localProcess, next, end int, results chan<- common.Result) processOutcome {
	stop := context.AfterFunc(ctx, proc.kill)
	defer stop()

	lines := make(chan string, 64)
	go readLines(proc.out, lines)

	outcome := processOutcome{next: next}
	wait := r.caseTimeout + r.startupAllowance
	watchdog := time.NewTimer(wait)
	defer watchdog.Stop()

	// For each test case, the runner prints whatever the test case writes, then
	// one result line, `[<status>] <case> (<ms>ms). Log:`, then the test case's
	// log lines, each indented. With --coverage, a `Code-coverage: [[<path>]]`
	// line comes after the last test case.
	var pending *common.Result // result whose log lines are being read
	var inflight []string      // output of the test case that has not reported yet
	var sinceResult []string   // output since the latest result line
	for lines != nil {
		select {
		case line, ok := <-lines:
			if !ok {
				lines = nil
				continue
			}

			// The coverage data is attached to the result, not added to its message.
			if r.coverage != nil {
				if path, ok := parseCoverageLine(line); ok {
					r.importCoverage(path, pending)
					continue
				}
			}
			sinceResult = append(sinceResult, line)

			// Indented lines that follow a result line are its log lines.
			if pending != nil && isLogLine(line) {
				pending.Message += line + "\n"
				continue
			}

			// Any other line ends the log lines, so the pending result is complete.
			reportIfDone(results, pending)
			pending = nil

			// Lines other than result lines are output of the next test case.
			res, isResult := r.resultFor(line, outcome.next, end)
			if !isResult {
				inflight = append(inflight, line)
				continue
			}

			// Report the test cases the runner skipped without a result line. That only
			// happens when its output contained something unexpected.
			r.reportNotRun(results, outcome.next, res.Index, fmt.Sprintf("the runner did not report a result for this test case, but reported %v\n", res.TestCase))

			// The result's message starts with the output of its test case, and the log
			// lines that follow are appended to it.
			res.Message = joinLines(inflight)
			pending = &res
			inflight, sinceResult = nil, nil
			outcome.next = res.Index + 1
			wait = r.caseTimeout
			watchdog.Reset(wait)
		case <-watchdog.C:
			outcome.timedOut = true
			outcome.waited = wait
			proc.kill()
		}
	}
	reportIfDone(results, pending)
	outcome.output = joinLines(sinceResult)
	outcome.exit = proc.wait()
	return outcome
}

// importCoverage imports coverage data from path into res.
func (r *cmdlineRunner) importCoverage(path string, res *common.Result) {
	if res == nil || r.fsReaderWriter == nil {
		return
	}
	coverage, err := r.coverage.Env.Import(path, r.fsReaderWriter)
	r.fsReaderWriter.Remove(path)
	if err != nil {
		res.Error = fmt.Errorf("could not import coverage data from '%v': %v", path, err)
		res.Status = common.Fail
		return
	}
	res.Coverage = coverage
}

// resultFor returns the result reported by line for one of the test cases in
// [next, end), or ok=false if line reports no such result.
func (r *cmdlineRunner) resultFor(line string, next, end int) (common.Result, bool) {
	word, testCase, duration, ok := parseResultLine(line)
	if !ok {
		return common.Result{}, false
	}
	i, found := r.index[testCase]
	if !found || i < next || i >= end {
		return common.Result{}, false
	}
	status, known := toStatus(word)
	res := common.Result{Index: i, TestCase: testCase, Status: status, Duration: duration}
	if !known {
		res.Error = fmt.Errorf("unknown status '%v'", word)
	}
	return res, true
}

// reportUnfinished reports the test case at index i, whose runner process ended
// or timed out before reporting its result.
func (r *cmdlineRunner) reportUnfinished(i int, outcome processOutcome) {
	res := common.Result{Index: i, TestCase: r.cases[i]}
	if outcome.timedOut {
		res.Status = common.Timeout
		res.Message = outcome.output + fmt.Sprintf("no result after %v, so the runner process was killed\n", outcome.waited)
	} else {
		res.Status = common.Crash
		res.Message = outcome.output + "the runner process ended before reporting a result for this test case: " + outcome.exit.description + "\n"
		if outcome.exit.success {
			res.Message += "an early exit with status 0 usually means that a test awaited a promise that never settles\n"
		}
	}
	r.report(res)
}

// report sends res to the results channel of the run.
func (r *cmdlineRunner) report(res common.Result) {
	r.results <- res
}

// reportNotRun sends a failure with message to results for each test case in
// [start, end).
func (r *cmdlineRunner) reportNotRun(results chan<- common.Result, start, end int, message string) {
	for i := start; i < end; i++ {
		results <- common.Result{Index: i, TestCase: r.cases[i], Status: common.Fail, Message: message}
	}
}

// reportIfDone sends res to results if it is not nil.
func reportIfDone(results chan<- common.Result, res *common.Result) {
	if res != nil {
		results <- *res
	}
}
