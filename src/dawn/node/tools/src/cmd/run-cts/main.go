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
	"io/ioutil"
	"math"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"
	"unicode/utf8"

	"dawn.googlesource.com/dawn/tools/src/utils"
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

var (
	colors  bool
	mainCtx context.Context
)

// ANSI escape sequences
const (
	escape       = "\u001B["
	positionLeft = escape + "0G"
	ansiReset    = escape + "0m"

	bold = escape + "1m"

	red     = escape + "31m"
	green   = escape + "32m"
	yellow  = escape + "33m"
	blue    = escape + "34m"
	magenta = escape + "35m"
	cyan    = escape + "36m"
	white   = escape + "37m"
)

type dawnNodeFlags []string

func (f *dawnNodeFlags) String() string {
	return fmt.Sprint(strings.Join(*f, ""))
}

func (f *dawnNodeFlags) Set(value string) error {
	// Multiple flags must be passed in individually:
	// -flag=a=b -dawn_node_flag=c=d
	*f = append(*f, value)
	return nil
}

func makeMainCtx() context.Context {
	ctx, cancel := context.WithCancel(context.Background())
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		sig := <-sigs
		fmt.Printf("Signal received: %v\n", sig)
		cancel()
	}()
	return ctx
}

func run() error {
	mainCtx = makeMainCtx()

	colors = os.Getenv("TERM") != "dumb" ||
		isatty.IsTerminal(os.Stdout.Fd()) ||
		isatty.IsCygwinTerminal(os.Stdout.Fd())
	if colors {
		if _, disable := os.LookupEnv("NO_COLOR"); disable {
			colors = false
		}
	}

	backendDefault := "default"
	if vkIcdFilenames := os.Getenv("VK_ICD_FILENAMES"); vkIcdFilenames != "" {
		backendDefault = "vulkan"
	}

	var dawnNode, cts, node, npx, resultsPath, expectationsPath, logFilename, backend string
	var printStdout, verbose, isolated, build bool
	var numRunners int
	var flags dawnNodeFlags
	flag.StringVar(&dawnNode, "dawn-node", "", "path to dawn.node module")
	flag.StringVar(&cts, "cts", defaultCtsPath(), "root directory of WebGPU CTS")
	flag.StringVar(&node, "node", defaultNodePath(), "path to node executable")
	flag.StringVar(&npx, "npx", "", "path to npx executable")
	flag.StringVar(&resultsPath, "output", "", "path to write test results file")
	flag.StringVar(&expectationsPath, "expect", "", "path to expectations file")
	flag.BoolVar(&printStdout, "print-stdout", false, "print the stdout and stderr from each test runner server")
	flag.BoolVar(&verbose, "verbose", false, "print extra information while testing")
	flag.BoolVar(&build, "build", true, "attempt to build the CTS before running")
	flag.BoolVar(&isolated, "isolate", false, "run each test in an isolated process")
	flag.BoolVar(&colors, "colors", colors, "enable / disable colors")
	flag.IntVar(&numRunners, "j", runtime.NumCPU()/2, "number of concurrent runners. 0 runs serially")
	flag.StringVar(&logFilename, "log", "", "path to log file of tests run and result")
	flag.Var(&flags, "flag", "flag to pass to dawn-node as flag=value. multiple flags must be passed in individually")
	flag.StringVar(&backend, "backend", backendDefault, "backend to use: default|null|webgpu|d3d11|d3d12|metal|vulkan|opengl|opengles."+
		" set to 'vulkan' if VK_ICD_FILENAMES environment variable is set, 'default' otherwise")
	flag.Parse()

	// Create a thread-safe, color supporting stdout wrapper.
	var stdout io.WriteCloser
	if colors {
		stdout = newMuxWriter(colorable.NewColorableStdout())
	} else {
		stdout = newMuxWriter(colorable.NewNonColorable(os.Stdout))
	}
	defer stdout.Close() // Required to flush the mux chan

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
		return fmt.Errorf("cannot find path to node. Specify with --node")
	}
	// Find npx
	if npx == "" {
		var err error
		npx, err = exec.LookPath("npx")
		if err != nil {
			npx = ""
		}
	}

	// Forward the backend to use, if specified.
	if backend != "default" {
		fmt.Fprintln(stdout, "Forcing backend to", backend)
		flags = append(flags, fmt.Sprint("dawn-backend=", backend))
	}

	// While running the CTS, always allow unsafe APIs so they can be tested.
	disableDawnFeaturesFound := false
	for i, flag := range flags {
		if strings.HasPrefix(flag, "disable-dawn-features=") {
			flags[i] = flag + ",disallow_unsafe_apis"
			disableDawnFeaturesFound = true
		}
	}
	if !disableDawnFeaturesFound {
		flags = append(flags, "disable-dawn-features=disallow_unsafe_apis")
	}

	r := runner{
		numRunners:  numRunners,
		printStdout: printStdout,
		verbose:     verbose,
		node:        node,
		npx:         npx,
		dawnNode:    dawnNode,
		cts:         cts,
		flags:       flags,
		results:     testcaseStatuses{},
		evalScript: func(main string) string {
			return fmt.Sprintf(`require('./src/common/tools/setup-ts-in-node.js');require('./src/common/runtime/%v.ts');`, main)
		},
		stdout: stdout,
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
		fmt.Fprintln(stdout, "failed to load cache from", cachePath, err)
	}
	defer cache.save(cachePath)

	// Scan the CTS source to determine the most recent change to the CTS source
	mostRecentSourceChange, err := r.scanSourceTimestamps(verbose)
	if err != nil {
		return fmt.Errorf("failed to scan source files for modified timestamps: %w", err)
	}

	ctsNeedsRebuild := mostRecentSourceChange.After(cache.BuildTimestamp) ||
		!isDir(filepath.Join(r.cts, "out-node"))
	if build {
		if verbose {
			fmt.Fprintln(stdout, "CTS needs rebuild:", ctsNeedsRebuild)
		}

		if npx != "" {
			if ctsNeedsRebuild {
				if err := r.buildCTS(verbose); err != nil {
					return fmt.Errorf("failed to build CTS: %w", err)
				}
				cache.BuildTimestamp = mostRecentSourceChange
			}
			// Use the prebuilt CTS (instead of using the `setup-ts-in-node` transpiler)
			r.evalScript = func(main string) string {
				return fmt.Sprintf(`require('./out-node/common/runtime/%v.js');`, main)
			}
		} else {
			fmt.Fprintln(stdout, "npx not found on PATH. Using runtime TypeScript transpilation (slow)")
		}
	}

	// If an expectations file was specified, load it.
	if expectationsPath != "" {
		if ex, err := loadExpectations(expectationsPath); err == nil {
			r.expectations = ex
		} else {
			return err
		}
	}

	if numRunners > 0 {
		// Find all the test cases that match the given queries.
		if err := r.gatherTestCases(query, verbose); err != nil {
			return fmt.Errorf("failed to gather test cases: %w", err)
		}

		if isolated {
			fmt.Fprintln(stdout, "Running in parallel isolated...")
			fmt.Fprintf(stdout, "Testing %d test cases...\n", len(r.testcases))
			if err := r.runParallelIsolated(); err != nil {
				return err
			}
		} else {
			fmt.Fprintln(stdout, "Running in parallel with server...")
			fmt.Fprintf(stdout, "Testing %d test cases...\n", len(r.testcases))
			if err := r.runParallelWithServer(); err != nil {
				return err
			}
		}
	} else {
		fmt.Fprintln(stdout, "Running serially...")
		if err := r.runSerially(query); err != nil {
			return err
		}
	}

	if resultsPath != "" {
		if err := saveExpectations(resultsPath, r.results); err != nil {
			return err
		}
	}

	return nil
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
	printStdout              bool
	verbose                  bool
	node, npx, dawnNode, cts string
	flags                    dawnNodeFlags
	evalScript               func(string) string
	testcases                []string
	expectations             testcaseStatuses
	results                  testcaseStatuses
	log                      logger
	stdout                   io.WriteCloser
}

// scanSourceTimestamps scans all the .js and .ts files in all subdirectories of
// r.cts, and returns the file with the most recent timestamp.
func (r *runner) scanSourceTimestamps(verbose bool) (time.Time, error) {
	if verbose {
		start := time.Now()
		fmt.Fprintln(r.stdout, "Scanning .js / .ts files for changes...")
		defer func() {
			fmt.Fprintln(r.stdout, "completed in", time.Since(start))
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
		fmt.Fprintln(r.stdout, "Building CTS...")
		defer func() {
			fmt.Fprintln(r.stdout, "completed in", time.Since(start))
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
		fmt.Fprintln(r.stdout, "Gathering test cases...")
		defer func() {
			fmt.Fprintln(r.stdout, "completed in", time.Since(start))
		}()
	}

	args := append([]string{
		"-e", r.evalScript("cmdline"),
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
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

// portListener implements io.Writer, monitoring written messages until a port
// is printed between '[[' ']]'. Once the port has been found, the parsed
// port number is written to the 'port' chan, and all subsequent writes are
// forwarded to writer.
type portListener struct {
	writer io.Writer
	buffer strings.Builder
	port   chan int
}

func newPortListener(w io.Writer) portListener {
	return portListener{w, strings.Builder{}, make(chan int)}
}

func (p *portListener) Write(data []byte) (n int, err error) {
	if p.port != nil {
		p.buffer.Write(data)
		str := p.buffer.String()

		idx := strings.Index(str, "[[")
		if idx < 0 {
			// Still waiting for the opening '[['
			return len(data), nil
		}

		str = str[idx+2:] // skip past '[['
		idx = strings.Index(str, "]]")
		if idx < 0 {
			// Still waiting for the closing ']]'
			return len(data), nil
		}

		port, err := strconv.Atoi(str[:idx])
		if err != nil {
			return 0, err
		}

		// Port found. Write it to the chan, and close the chan.
		p.port <- port
		close(p.port)
		p.port = nil

		str = strings.TrimRight(str[idx+2:], " \n")
		if len(str) == 0 {
			return len(data), nil
		}
		// Write out trailing text after the ']]'
		return p.writer.Write([]byte(str))
	}

	// Port has been found. Just forward the rest of the data to p.writer
	return p.writer.Write(data)
}

// prefixWriter is an io.Writer that prefixes each write with a prefix string
type prefixWriter struct {
	prefix  string
	writer  io.Writer
	midLine bool
}

func (p *prefixWriter) Write(data []byte) (int, error) {
	lines := strings.Split(string(data), "\n")
	buf := strings.Builder{}
	for i, line := range lines {
		if line == "" && i == len(lines)-1 {
			break
		}
		buf.WriteString(p.prefix)
		buf.WriteString(line)
		buf.WriteString("\n")
	}
	if _, err := p.writer.Write([]byte(buf.String())); err != nil {
		return 0, err
	}
	return len(data), nil
}

// runParallelWithServer() starts r.numRunners instances of the CTS server test
// runner, and issues test run requests to those servers, concurrently.
func (r *runner) runParallelWithServer() error {
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
	wg := &sync.WaitGroup{}
	for i := 0; i < r.numRunners; i++ {
		id := i
		wg.Add(1)
		go func() {
			defer wg.Done()
			if err := r.runServer(id, caseIndices, results); err != nil {
				results <- result{
					status: fail,
					error:  fmt.Errorf("Test server error: %w", err),
				}
			}
		}()
	}

	r.streamResults(wg, results)
	return nil
}

// runServer starts a test runner server instance, takes case indices from
// caseIndices, and requests the server run the test with the given index.
// The result of the test run is written to the results chan.
// Once the caseIndices chan has been closed, the server is stopped and
// runServer returns.
func (r *runner) runServer(id int, caseIndices <-chan int, results chan<- result) error {
	var port int
	testCaseLog := &bytes.Buffer{}

	stopServer := func() {}
	startServer := func() error {
		args := []string{
			"-e", r.evalScript("server"), // Evaluate 'eval'.
			"--",
			// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
			// and slices away the first two arguments. When running with '-e', args
			// start at 1, so just inject a placeholder argument.
			"placeholder-arg",
			// Actual arguments begin here
			"--gpu-provider", r.dawnNode,
		}
		for _, f := range r.flags {
			args = append(args, "--gpu-provider-flag", f)
		}

		ctx := mainCtx
		cmd := exec.CommandContext(ctx, r.node, args...)

		writer := io.Writer(testCaseLog)
		if r.printStdout {
			pw := &prefixWriter{
				prefix: fmt.Sprintf("[%d] ", id),
				writer: r.stdout,
			}
			writer = io.MultiWriter(pw, writer)
		}

		pl := newPortListener(writer)

		cmd.Dir = r.cts
		cmd.Stdout = &pl
		cmd.Stderr = &pl

		err := cmd.Start()
		if err != nil {
			return fmt.Errorf("failed to start test runner server: %v", err)
		}

		select {
		case port = <-pl.port:
			return nil // success
		case <-time.After(time.Second * 10):
			return fmt.Errorf("timeout waiting for server port:\n%v", pl.buffer.String())
		case <-ctx.Done(): // cancelled
			return ctx.Err()
		}

		return nil
	}
	stopServer = func() {
		if port > 0 {
			go http.Post(fmt.Sprintf("http://localhost:%v/terminate", port), "", &bytes.Buffer{})
			time.Sleep(time.Millisecond * 100)
			port = 0
		}
	}

	for idx := range caseIndices {
		testCaseLog.Reset() // Clear the log for this test case

		if port == 0 {
			if err := startServer(); err != nil {
				return err
			}
		}

		res := result{index: idx, testcase: r.testcases[idx]}

		type Response struct {
			Status  string
			Message string
		}
		postResp, err := http.Post(fmt.Sprintf("http://localhost:%v/run?%v", port, r.testcases[idx]), "", &bytes.Buffer{})
		if err != nil {
			res.error = fmt.Errorf("server POST failure. Restarting server... This can happen when there is a crash. Try running with --isolate.")
			res.status = fail
			results <- res
			stopServer()
			continue
		}

		if postResp.StatusCode == http.StatusOK {
			var resp Response
			if err := json.NewDecoder(postResp.Body).Decode(&resp); err != nil {
				res.error = fmt.Errorf("server response decode failure")
				res.status = fail
				results <- res
				continue
			}

			switch resp.Status {
			case "pass":
				res.status = pass
				res.message = resp.Message + testCaseLog.String()
			case "warn":
				res.status = warn
				res.message = resp.Message + testCaseLog.String()
			case "fail":
				res.status = fail
				res.message = resp.Message + testCaseLog.String()
			case "skip":
				res.status = skip
				res.message = resp.Message + testCaseLog.String()
			default:
				res.status = fail
				res.error = fmt.Errorf("unknown status: '%v'", resp.Status)
			}
		} else {
			msg, err := ioutil.ReadAll(postResp.Body)
			if err != nil {
				msg = []byte(err.Error())
			}
			res.status = fail
			res.error = fmt.Errorf("server error: %v", string(msg))
		}
		results <- res
	}

	stopServer()
	return nil
}

// runParallelIsolated() calls the CTS command-line test runner to run each
// testcase in a separate process. This reduces possibility of state leakage
// between tests.
// Up to r.numRunners tests will be run concurrently.
func (r *runner) runParallelIsolated() error {
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
	wg := &sync.WaitGroup{}
	for i := 0; i < r.numRunners; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for idx := range caseIndices {
				res := r.runTestcase(r.testcases[idx])
				res.index = idx
				results <- res
			}
		}()
	}

	r.streamResults(wg, results)
	return nil
}

// streamResults reads from the chan 'results', printing the results in test-id
// sequential order. Once the WaitGroup 'wg' is complete, streamResults() will
// automatically close the 'results' chan.
// Once all the results have been printed, a summary will be printed and the
// function will return.
func (r *runner) streamResults(wg *sync.WaitGroup, results chan result) {
	// Create another goroutine to close the results chan when all the runner
	// goroutines have finished.
	start := time.Now()
	var timeTaken time.Duration
	go func() {
		wg.Wait()
		timeTaken = time.Since(start)
		close(results)
	}()

	// Total number of tests, test counts binned by status
	numTests, numByExpectedStatus := len(r.testcases), map[expectedStatus]int{}

	// Helper function for printing a progress bar.
	lastStatusUpdate, animFrame := time.Now(), 0
	updateProgress := func() {
		fmt.Fprint(r.stdout, ansiProgressBar(animFrame, numTests, numByExpectedStatus))
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
		r.results[res.testcase] = res.status
		expected := r.expectations[res.testcase]
		exStatus := expectedStatus{
			status:   res.status,
			expected: expected == res.status,
		}
		numByExpectedStatus[exStatus] = numByExpectedStatus[exStatus] + 1
		name := res.testcase
		if r.verbose ||
			res.error != nil ||
			(exStatus.status != pass && exStatus.status != skip && !exStatus.expected) {
			buf := &bytes.Buffer{}
			fmt.Fprint(buf, statusColor[res.status])
			if res.message != "" {
				fmt.Fprintf(buf, "%v - %v:\n", name, res.status)
				fmt.Fprintf(buf, ansiReset)
				fmt.Fprintf(buf, "%v", res.message)
			} else {
				fmt.Fprintf(buf, "%v - %v", name, res.status)
			}
			if expected != "" {
				fmt.Fprintf(buf, " [%v -> %v]", expected, res.status)
			}
			fmt.Fprintln(buf)
			if res.error != nil {
				fmt.Fprintln(buf, res.error)
			}
			fmt.Fprint(buf, ansiReset)
			fmt.Fprint(r.stdout, buf.String())
			updateProgress()
		}
		if time.Since(lastStatusUpdate) > progressUpdateRate {
			updateProgress()
		}
	}
	fmt.Fprint(r.stdout, ansiProgressBar(animFrame, numTests, numByExpectedStatus))

	// All done. Print final stats.
	fmt.Fprintf(r.stdout, "\nCompleted in %v\n", timeTaken)

	var numExpectedByStatus map[status]int
	if r.expectations != nil {
		// The status of each testcase that was run
		numExpectedByStatus = map[status]int{}
		for t, s := range r.expectations {
			if _, wasTested := r.results[t]; wasTested {
				numExpectedByStatus[s] = numExpectedByStatus[s] + 1
			}
		}
	}

	for _, s := range statuses {
		// number of tests, just run, that resulted in the given status
		numByStatus := numByExpectedStatus[expectedStatus{s, true}] +
			numByExpectedStatus[expectedStatus{s, false}]
		// difference in number of tests that had the given status from the
		// expected number (taken from the expectations file)
		diffFromExpected := 0
		if numExpectedByStatus != nil {
			diffFromExpected = numByStatus - numExpectedByStatus[s]
		}
		if numByStatus == 0 && diffFromExpected == 0 {
			continue
		}

		fmt.Fprint(r.stdout, bold, statusColor[s])
		fmt.Fprint(r.stdout, alignRight(strings.ToUpper(string(s))+": ", 10))
		fmt.Fprint(r.stdout, ansiReset)
		if numByStatus > 0 {
			fmt.Fprint(r.stdout, bold)
		}
		fmt.Fprint(r.stdout, alignLeft(numByStatus, 10))
		fmt.Fprint(r.stdout, ansiReset)
		fmt.Fprint(r.stdout, alignRight("("+percentage(numByStatus, numTests)+")", 6))

		if diffFromExpected != 0 {
			fmt.Fprint(r.stdout, bold, " [")
			fmt.Fprintf(r.stdout, "%+d", diffFromExpected)
			fmt.Fprint(r.stdout, ansiReset, "]")
		}
		fmt.Fprintln(r.stdout)
	}

}

// runSerially() calls the CTS test runner to run the test query in a single
// process.
// TODO(bclayton): Support comparing against r.expectations
func (r *runner) runSerially(query string) error {
	start := time.Now()
	result := r.runTestcase(query)
	timeTaken := time.Since(start)

	if r.verbose {
		fmt.Fprintln(r.stdout, result)
	}
	fmt.Fprintln(r.stdout, "Status:", result.status)
	fmt.Fprintln(r.stdout, "Completed in", timeTaken)
	return nil
}

// status is an enumerator of test result status
type status string

const (
	pass    status = "pass"
	warn    status = "warn"
	fail    status = "fail"
	skip    status = "skip"
	timeout status = "timeout"
)

// All the status types
var statuses = []status{pass, warn, fail, skip, timeout}

var statusColor = map[status]string{
	pass:    green,
	warn:    yellow,
	skip:    cyan,
	timeout: yellow,
	fail:    red,
}

// expectedStatus is a test status, along with a boolean to indicate whether the
// status matches the test expectations
type expectedStatus struct {
	status   status
	expected bool
}

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
func (r *runner) runTestcase(query string) result {
	ctx, cancel := context.WithTimeout(mainCtx, testTimeout)
	defer cancel()

	args := []string{
		"-e", r.evalScript("cmdline"), // Evaluate 'eval'.
		"--",
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		// Actual arguments begin here
		"--gpu-provider", r.dawnNode,
		"--verbose",
		"--quiet",
	}
	for _, f := range r.flags {
		args = append(args, "--gpu-provider-flag", f)
	}
	args = append(args, query)

	cmd := exec.CommandContext(ctx, r.node, args...)
	cmd.Dir = r.cts

	var buf bytes.Buffer
	cmd.Stdout = &buf
	cmd.Stderr = &buf

	err := cmd.Run()
	msg := buf.String()
	switch {
	case errors.Is(err, context.DeadlineExceeded):
		return result{testcase: query, status: timeout, message: msg}
	case strings.Contains(msg, "[fail]"):
		return result{testcase: query, status: fail, message: msg}
	case strings.Contains(msg, "[warn]"):
		return result{testcase: query, status: warn, message: msg}
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

// alignLeft returns the string of 'val' padded so that it is aligned left in
// a column of the given width
func alignLeft(val interface{}, width int) string {
	s := fmt.Sprint(val)
	padding := width - utf8.RuneCountInString(s)
	if padding < 0 {
		return s
	}
	return s + strings.Repeat(" ", padding)
}

// alignRight returns the string of 'val' padded so that it is aligned right in
// a column of the given width
func alignRight(val interface{}, width int) string {
	s := fmt.Sprint(val)
	padding := width - utf8.RuneCountInString(s)
	if padding < 0 {
		return s
	}
	return strings.Repeat(" ", padding) + s
}

// ansiProgressBar returns a string with an ANSI-colored progress bar, providing
// realtime information about the status of the CTS run.
// Note: We'll want to skip this if !isatty or if we're running on windows.
func ansiProgressBar(animFrame int, numTests int, numByExpectedStatus map[expectedStatus]int) string {
	const barWidth = 50

	animSymbols := []rune{'⣾', '⣽', '⣻', '⢿', '⡿', '⣟', '⣯', '⣷'}
	blockSymbols := []rune{'▏', '▎', '▍', '▌', '▋', '▊', '▉'}

	numBlocksPrinted := 0

	buf := &strings.Builder{}
	fmt.Fprint(buf, string(animSymbols[animFrame%len(animSymbols)]), " [")
	animFrame++

	numFinished := 0

	for _, status := range statuses {
		for _, expected := range []bool{true, false} {
			color := statusColor[status]
			if expected {
				color += bold
			}

			num := numByExpectedStatus[expectedStatus{status, expected}]
			numFinished += num
			statusFrac := float64(num) / float64(numTests)
			fNumBlocks := barWidth * statusFrac
			fmt.Fprint(buf, color)
			numBlocks := int(math.Ceil(fNumBlocks))
			if expected {
				if numBlocks > 1 {
					fmt.Fprint(buf, strings.Repeat(string("░"), numBlocks))
				}
			} else {
				if numBlocks > 1 {
					fmt.Fprint(buf, strings.Repeat(string("▉"), numBlocks))
				}
				if numBlocks > 0 {
					frac := fNumBlocks - math.Floor(fNumBlocks)
					symbol := blockSymbols[int(math.Round(frac*float64(len(blockSymbols)-1)))]
					fmt.Fprint(buf, string(symbol))
				}
			}
			numBlocksPrinted += numBlocks
		}
	}

	if barWidth > numBlocksPrinted {
		fmt.Fprint(buf, strings.Repeat(string(" "), barWidth-numBlocksPrinted))
	}
	fmt.Fprint(buf, ansiReset)
	fmt.Fprint(buf, "] ", percentage(numFinished, numTests))

	if colors {
		// move cursor to start of line so the bar is overridden
		fmt.Fprint(buf, positionLeft)
	} else {
		// cannot move cursor, so newline
		fmt.Fprintln(buf)
	}

	return buf.String()
}

// testcaseStatus is a pair of testcase name and result status
// Intended to be serialized for expectations files.
type testcaseStatus struct {
	Testcase string
	Status   status
}

// testcaseStatuses is a map of testcase to test status
type testcaseStatuses map[string]status

// loadExpectations loads the test expectations from path
func loadExpectations(path string) (testcaseStatuses, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("failed to open expectations file: %w", err)
	}
	defer f.Close()

	statuses := []testcaseStatus{}
	if err := json.NewDecoder(f).Decode(&statuses); err != nil {
		return nil, fmt.Errorf("failed to read expectations file: %w", err)
	}

	out := make(testcaseStatuses, len(statuses))
	for _, s := range statuses {
		out[s.Testcase] = s.Status
	}
	return out, nil
}

// saveExpectations saves the test results 'ex' as an expectations file to path
func saveExpectations(path string, ex testcaseStatuses) error {
	f, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("failed to create expectations file: %w", err)
	}
	defer f.Close()

	statuses := make([]testcaseStatus, 0, len(ex))
	for testcase, status := range ex {
		statuses = append(statuses, testcaseStatus{testcase, status})
	}
	sort.Slice(statuses, func(i, j int) bool { return statuses[i].Testcase < statuses[j].Testcase })

	e := json.NewEncoder(f)
	e.SetIndent("", "  ")
	if err := e.Encode(&statuses); err != nil {
		return fmt.Errorf("failed to save expectations file: %w", err)
	}

	return nil
}

// defaultNodePath looks for the node binary, first in dawn's third_party
// directory, falling back to PATH. This is used as the default for the --node
// command line flag.
func defaultNodePath() string {
	if dawnRoot := utils.DawnRoot(); dawnRoot != "" {
		node := filepath.Join(dawnRoot, "third_party/node")
		if info, err := os.Stat(node); err == nil && info.IsDir() {
			path := ""
			switch fmt.Sprintf("%v/%v", runtime.GOOS, runtime.GOARCH) { // See `go tool dist list`
			case "darwin/amd64":
				path = filepath.Join(node, "node-darwin-x64/bin/node")
			case "darwin/arm64":
				path = filepath.Join(node, "node-darwin-arm64/bin/node")
			case "linux/amd64":
				path = filepath.Join(node, "node-linux-x64/bin/node")
			case "windows/amd64":
				path = filepath.Join(node, "node.exe")
			}
			if _, err := os.Stat(path); err == nil {
				return path
			}
		}
	}

	if path, err := exec.LookPath("node"); err == nil {
		return path
	}

	return ""
}

// defaultCtsPath looks for the webgpu-cts directory in dawn's third_party
// directory. This is used as the default for the --cts command line flag.
func defaultCtsPath() string {
	if dawnRoot := utils.DawnRoot(); dawnRoot != "" {
		cts := filepath.Join(dawnRoot, "third_party/webgpu-cts")
		if info, err := os.Stat(cts); err == nil && info.IsDir() {
			return cts
		}
	}
	return ""
}

type muxWriter struct {
	data chan []byte
	err  chan error
}

// muxWriter returns a thread-safe io.WriteCloser, that writes to w
func newMuxWriter(w io.Writer) *muxWriter {
	m := muxWriter{
		data: make(chan []byte, 256),
		err:  make(chan error, 1),
	}
	go func() {
		defer close(m.err)
		for data := range m.data {
			_, err := w.Write(data)
			if err != nil {
				m.err <- err
				return
			}
		}
		m.err <- nil
	}()
	return &m
}

func (w *muxWriter) Write(data []byte) (n int, err error) {
	w.data <- append([]byte{}, data...)
	return len(data), nil
}

func (w *muxWriter) Close() error {
	close(w.data)
	return <-w.err
}
