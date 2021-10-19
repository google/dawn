// Copyright 2021 The Dawn Authors
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

// run-cts is a tool used to run the WebGPU CTS using the Dawn module for NodeJS
package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"time"

	"github.com/mattn/go-colorable"
	"github.com/mattn/go-isatty"
)

const (
	testTimeout = time.Minute
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
run-cts is a tool used to run the WebGPU CTS using the Dawn module for NodeJS

Usage:
  run-cts --dawn-node=<path to dawn.node> --cts=<path to WebGPU CTS> [test-query]`)
	os.Exit(1)
}

var colors bool
var stdout io.Writer

func run() error {
	colors = os.Getenv("TERM") != "dumb" ||
		isatty.IsTerminal(os.Stdout.Fd()) ||
		isatty.IsCygwinTerminal(os.Stdout.Fd())
	if colors {
		if _, disable := os.LookupEnv("NO_COLOR"); disable {
			colors = false
		}
	}

	var dawnNode, cts, node, npx, logFilename string
	var verbose, build bool
	var numRunners int
	flag.StringVar(&dawnNode, "dawn-node", "", "path to dawn.node module")
	flag.StringVar(&cts, "cts", "", "root directory of WebGPU CTS")
	flag.StringVar(&node, "node", "", "path to node executable")
	flag.StringVar(&npx, "npx", "", "path to npx executable")
	flag.BoolVar(&verbose, "verbose", false, "print extra information while testing")
	flag.BoolVar(&build, "build", true, "attempt to build the CTS before running")
	flag.BoolVar(&colors, "colors", colors, "enable / disable colors")
	flag.IntVar(&numRunners, "j", runtime.NumCPU(), "number of concurrent runners. 0 runs serially")
	flag.StringVar(&logFilename, "log", "", "path to log file of tests run and result")
	flag.Parse()

	if colors {
		stdout = colorable.NewColorableStdout()
	} else {
		stdout = colorable.NewNonColorable(os.Stdout)
	}

	// Check mandatory arguments
	if dawnNode == "" || cts == "" {
		showUsage()
	}
	if !isFile(dawnNode) {
		return fmt.Errorf("'%v' is not a file", dawnNode)
	}
	if !isDir(cts) {
		return fmt.Errorf("'%v' is not a directory", cts)
	}

	// Make paths absolute
	for _, path := range []*string{&dawnNode, &cts} {
		abs, err := filepath.Abs(*path)
		if err != nil {
			return fmt.Errorf("unable to get absolute path for '%v'", *path)
		}
		*path = abs
	}

	// The test query is the optional unnamed argument
	query := "webgpu:*"
	switch len(flag.Args()) {
	case 0:
	case 1:
		query = flag.Args()[0]
	default:
		return fmt.Errorf("only a single query can be provided")
	}

	// Find node
	if node == "" {
		var err error
		node, err = exec.LookPath("node")
		if err != nil {
			return fmt.Errorf("add node to PATH or specify with --node")
		}
	}
	// Find npx
	if npx == "" {
		var err error
		npx, err = exec.LookPath("npx")
		if err != nil {
			npx = ""
		}
	}

	r := runner{
		numRunners: numRunners,
		verbose:    verbose,
		node:       node,
		npx:        npx,
		dawnNode:   dawnNode,
		cts:        cts,
		evalScript: `require('./src/common/tools/setup-ts-in-node.js');
		  require('./src/common/runtime/cmdline.ts');`,
	}

	if logFilename != "" {
		writer, err := os.Create(logFilename)
		if err != nil {
			return fmt.Errorf("failed to open log '%v': %w", logFilename, err)
		}
		defer writer.Close()
		r.log = newLogger(writer)
	}

	cache := cache{}
	cachePath := dawnNode + ".runcts.cache"
	if err := cache.load(cachePath); err != nil && verbose {
		fmt.Println("failed to load cache from", cachePath, err)
	}
	defer cache.save(cachePath)

	// Scan the CTS source to determine the most recent change to the CTS source
	mostRecentSourceChange, err := r.scanSourceTimestamps(verbose)
	if err != nil {
		return fmt.Errorf("failed to scan source files for modified timestamps: %w", err)
	}

	ctsNeedsRebuild := mostRecentSourceChange.After(cache.BuildTimestamp)
	if build {
		if verbose {
			fmt.Println("CTS needs rebuild:", ctsNeedsRebuild)
		}

		if npx != "" {
			if ctsNeedsRebuild {
				if err := r.buildCTS(verbose); err != nil {
					return fmt.Errorf("failed to build CTS: %w", err)
				}
				cache.BuildTimestamp = mostRecentSourceChange
			}
			// Use the prebuilt CTS (instead of using the `setup-ts-in-node` transpiler)
			r.evalScript = `require('./out-node/common/runtime/cmdline.js');`
		} else {
			fmt.Println("npx not found on PATH. Using runtime TypeScript transpilation (slow)")
		}
	}

	if numRunners > 0 {
		// Find all the test cases that match the given queries.
		if err := r.gatherTestCases(query, verbose); err != nil {
			return fmt.Errorf("failed to gather test cases: %w", err)
		}

		fmt.Printf("Testing %d test cases...\n", len(r.testcases))
		return r.runParallel()
	}

	fmt.Println("Running serially...")
	return r.runSerially(query)
}

type logger struct {
	writer        io.Writer
	idx           int
	resultByIndex map[int]result
}

// newLogger creates a new logger instance.
func newLogger(writer io.Writer) logger {
	return logger{writer, 0, map[int]result{}}
}

// logResult writes the test results to the log file in sequential order.
// logResult should be called whenever a new test result becomes available.
func (l *logger) logResults(res result) {
	if l.writer == nil {
		return
	}
	l.resultByIndex[res.index] = res
	for {
		logRes, ok := l.resultByIndex[l.idx]
		if !ok {
			break
		}
		fmt.Fprintf(l.writer, "%v [%v]\n", logRes.testcase, logRes.status)
		l.idx++
	}
}

// Cache holds cached information between runs to optimize runs
type cache struct {
	BuildTimestamp time.Time
}

// load loads the cache information from the JSON file at path
func (c *cache) load(path string) error {
	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()
	return json.NewDecoder(f).Decode(c)
}

// save saves the cache information to the JSON file at path
func (c *cache) save(path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()
	return json.NewEncoder(f).Encode(c)
}

type runner struct {
	numRunners               int
	verbose                  bool
	node, npx, dawnNode, cts string
	evalScript               string
	testcases                []string
	log                      logger
}

// scanSourceTimestamps scans all the .js and .ts files in all subdirectories of
// r.cts, and returns the file with the most recent timestamp.
func (r *runner) scanSourceTimestamps(verbose bool) (time.Time, error) {
	if verbose {
		start := time.Now()
		fmt.Println("Scanning .js / .ts files for changes...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	dir := filepath.Join(r.cts, "src")

	mostRecentChange := time.Time{}
	err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		switch filepath.Ext(path) {
		case ".ts", ".js":
			if info.ModTime().After(mostRecentChange) {
				mostRecentChange = info.ModTime()
			}
		}
		return nil
	})
	if err != nil {
		return time.Time{}, err
	}
	return mostRecentChange, nil
}

// buildCTS calls `npx grunt run:build-out-node` in the CTS directory to compile
// the TypeScript files down to JavaScript. Doing this once ahead of time can be
// much faster than dynamically transpiling when there are many tests to run.
func (r *runner) buildCTS(verbose bool) error {
	if verbose {
		start := time.Now()
		fmt.Println("Building CTS...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	cmd := exec.Command(r.npx, "grunt", "run:build-out-node")
	cmd.Dir = r.cts
	out, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("%w: %v", err, string(out))
	}
	return nil
}

// gatherTestCases() queries the CTS for all test cases that match the given
// query. On success, gatherTestCases() populates r.testcases.
func (r *runner) gatherTestCases(query string, verbose bool) error {
	if verbose {
		start := time.Now()
		fmt.Println("Gathering test cases...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	args := append([]string{
		"-e", r.evalScript,
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a dummy argument.
		"dummy-arg",
		"--list",
	}, query)

	cmd := exec.Command(r.node, args...)
	cmd.Dir = r.cts
	out, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("%w\n%v", err, string(out))
	}

	tests := filterTestcases(strings.Split(string(out), "\n"))
	r.testcases = tests
	return nil
}

// runParallel() calls the CTS test runner to run each testcase in a separate
// process.
// Up to r.numRunners tests will be run concurrently.
func (r *runner) runParallel() error {
	// Create a chan of test indices.
	// This will be read by the test runner goroutines.
	caseIndices := make(chan int, len(r.testcases))
	for i := range r.testcases {
		caseIndices <- i
	}
	close(caseIndices)

	// Create a chan for the test results.
	// This will be written to by the test runner goroutines.
	results := make(chan result, len(r.testcases))

	// Spin up the test runner goroutines
	start := time.Now()
	wg := sync.WaitGroup{}
	for i := 0; i < r.numRunners; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for idx := range caseIndices {
				res := r.runTestcase(r.testcases[idx], false)
				res.index = idx
				results <- res
			}
		}()
	}

	// Create another goroutine to close the results chan when all the runner
	// goroutines have finished.
	var timeTaken time.Duration
	go func() {
		wg.Wait()
		timeTaken = time.Since(start)
		close(results)
	}()

	// Total number of tests, test counts binned by status
	numTests, numByStatus := len(r.testcases), map[status]int{}

	// Helper function for printing a progress bar.
	lastStatusUpdate, animFrame := time.Now(), 0
	updateProgress := func() {
		printANSIProgressBar(animFrame, numTests, numByStatus)
		animFrame++
		lastStatusUpdate = time.Now()
	}

	// Pull test results as they become available.
	// Update the status counts, and print any failures (or all test results if --verbose)
	progressUpdateRate := time.Millisecond * 10
	if !colors {
		// No colors == no cursor control. Reduce progress updates so that
		// we're not printing endless progress bars.
		progressUpdateRate = time.Second
	}

	for res := range results {
		r.log.logResults(res)

		numByStatus[res.status] = numByStatus[res.status] + 1
		name := res.testcase
		if r.verbose || (res.status != pass && res.status != skip) {
			fmt.Printf("%v - %v: %v\n", name, res.status, res.message)
			updateProgress()
		}
		if time.Since(lastStatusUpdate) > progressUpdateRate {
			updateProgress()
		}
	}
	printANSIProgressBar(animFrame, numTests, numByStatus)

	// All done. Print final stats.
	fmt.Printf(`
Completed in %v

pass:    %v (%v)
fail:    %v (%v)
skip:    %v (%v)
timeout: %v (%v)
`,
		timeTaken,
		numByStatus[pass], percentage(numByStatus[pass], numTests),
		numByStatus[fail], percentage(numByStatus[fail], numTests),
		numByStatus[skip], percentage(numByStatus[skip], numTests),
		numByStatus[timeout], percentage(numByStatus[timeout], numTests),
	)
	return nil
}

// runSerially() calls the CTS test runner to run the test query in a single
// process.
func (r *runner) runSerially(query string) error {
	start := time.Now()
	result := r.runTestcase(query, true)
	timeTaken := time.Since(start)

	fmt.Println("Completed in", timeTaken)
	fmt.Println(result)
	return nil
}

// status is an enumerator of test result status
type status string

const (
	pass    status = "pass"
	fail    status = "fail"
	skip    status = "skip"
	timeout status = "timeout"
)

// result holds the information about a completed test
type result struct {
	index    int
	testcase string
	status   status
	message  string
	error    error
}

// runTestcase() runs the CTS testcase with the given query, returning the test
// result.
func (r *runner) runTestcase(query string, printToStout bool) result {
	ctx, cancel := context.WithTimeout(context.Background(), testTimeout)
	defer cancel()

	eval := r.evalScript
	args := append([]string{
		"-e", eval, // Evaluate 'eval'.
		"--",
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a dummy argument.
		"dummy-arg",
		// Actual arguments begin here
		"--gpu-provider", r.dawnNode,
		"--verbose",
	}, query)

	cmd := exec.CommandContext(ctx, r.node, args...)
	cmd.Dir = r.cts

	var buf bytes.Buffer
	if printToStout {
		cmd.Stdout = io.MultiWriter(&buf, os.Stdout)
		cmd.Stderr = io.MultiWriter(&buf, os.Stderr)
	} else {
		cmd.Stdout = &buf
		cmd.Stderr = &buf
	}

	err := cmd.Run()
	msg := buf.String()
	switch {
	case errors.Is(err, context.DeadlineExceeded):
		return result{testcase: query, status: timeout, message: msg}
	case strings.Contains(msg, "[fail]"):
		return result{testcase: query, status: fail, message: msg}
	case strings.Contains(msg, "[skip]"):
		return result{testcase: query, status: skip, message: msg}
	case strings.Contains(msg, "[pass]"), err == nil:
		return result{testcase: query, status: pass, message: msg}
	}
	return result{testcase: query, status: fail, message: fmt.Sprint(msg, err), error: err}
}

// filterTestcases returns in with empty strings removed
func filterTestcases(in []string) []string {
	out := make([]string, 0, len(in))
	for _, c := range in {
		if c != "" {
			out = append(out, c)
		}
	}
	return out
}

// percentage returns the percentage of n out of total as a string
func percentage(n, total int) string {
	if total == 0 {
		return "-"
	}
	f := float64(n) / float64(total)
	return fmt.Sprintf("%.1f%c", f*100.0, '%')
}

// isDir returns true if the path resolves to a directory
func isDir(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}
	return s.IsDir()
}

// isFile returns true if the path resolves to a file
func isFile(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}
	return !s.IsDir()
}

// printANSIProgressBar prints a colored progress bar, providing realtime
// information about the status of the CTS run.
// Note: We'll want to skip this if !isatty or if we're running on windows.
func printANSIProgressBar(animFrame int, numTests int, numByStatus map[status]int) {
	const (
		barWidth = 50

		escape       = "\u001B["
		positionLeft = escape + "0G"
		red          = escape + "31m"
		green        = escape + "32m"
		yellow       = escape + "33m"
		blue         = escape + "34m"
		magenta      = escape + "35m"
		cyan         = escape + "36m"
		white        = escape + "37m"
		reset        = escape + "0m"
	)

	animSymbols := []rune{'⣾', '⣽', '⣻', '⢿', '⡿', '⣟', '⣯', '⣷'}
	blockSymbols := []rune{'▏', '▎', '▍', '▌', '▋', '▊', '▉'}

	numBlocksPrinted := 0

	fmt.Fprint(stdout, string(animSymbols[animFrame%len(animSymbols)]), " [")
	animFrame++

	numFinished := 0

	for _, ty := range []struct {
		status status
		color  string
	}{{pass, green}, {skip, blue}, {timeout, yellow}, {fail, red}} {
		num := numByStatus[ty.status]
		numFinished += num
		statusFrac := float64(num) / float64(numTests)
		fNumBlocks := barWidth * statusFrac
		fmt.Fprint(stdout, ty.color)
		numBlocks := int(math.Ceil(fNumBlocks))
		if numBlocks > 1 {
			fmt.Print(strings.Repeat(string("▉"), numBlocks))
		}
		if numBlocks > 0 {
			frac := fNumBlocks - math.Floor(fNumBlocks)
			symbol := blockSymbols[int(math.Round(frac*float64(len(blockSymbols)-1)))]
			fmt.Print(string(symbol))
		}
		numBlocksPrinted += numBlocks
	}

	if barWidth > numBlocksPrinted {
		fmt.Print(strings.Repeat(string(" "), barWidth-numBlocksPrinted))
	}
	fmt.Fprint(stdout, reset)
	fmt.Print("] ", percentage(numFinished, numTests))

	if colors {
		// move cursor to start of line so the bar is overridden
		fmt.Fprint(stdout, positionLeft)
	} else {
		// cannot move cursor, so newline
		fmt.Println()
	}
}
