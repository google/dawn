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
	"fmt"
	"io"
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"
	"time"
)

// fakeRunnerEnv makes the test binary act as a CTS command-line runner.
const fakeRunnerEnv = "RUN_CTS_FAKE_RUNNER"

func TestMain(m *testing.M) {
	if os.Getenv(fakeRunnerEnv) != "" {
		os.Exit(fakeRunner(os.Args[1:]))
	}
	os.Exit(m.Run())
}

// fakeRunner speaks the CTS command-line runner's line protocol for both
// dawn_standalone_cts_runner and node cmdline.js. out-node/cases.txt in the
// CTS directory lists every test case, and a query ending in '*' matches the
// test cases that start with the rest of it. What a test case does depends on
// the words in its name:
//   - crash: the process kills itself
//   - stall: the process exits with status 0, as the runner does when a test
//     awaits a promise that never settles
//   - hang: it never finishes
//   - noexit: it passes, and the process never exits
func fakeRunner(args []string) int {
	ctsDir, queryFile := ".", ""
	queries := []string{}
	list := false
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "--cts-dir":
			i++
			ctsDir = args[i]
		case "-q", "--query":
			i++
			queries = append(queries, args[i])
		case "--query-file":
			i++
			queryFile = args[i]
		case "-e", "--dawn-flag", "--gpu-provider", "--gpu-provider-flag":
			i++
		case "--", "placeholder-arg":
		case "--list":
			list = true
		default:
			if !strings.HasPrefix(args[i], "-") {
				queries = append(queries, args[i])
			}
		}
	}

	if queryFile == "-" {
		data, _ := io.ReadAll(os.Stdin)
		queries = append(queries, nonEmptyLines(string(data))...)
	}

	data, err := os.ReadFile(filepath.Join(ctsDir, "out-node", "cases.txt"))
	if err != nil {
		fmt.Println(err)
		return 1
	}
	all := nonEmptyLines(string(data))
	cases := []string{}
	for _, query := range queries {
		matched := false
		for _, testCase := range all {
			if testCase == query || (strings.HasSuffix(query, "*") && strings.HasPrefix(testCase, strings.TrimSuffix(query, "*"))) {
				cases = append(cases, testCase)
				matched = true
			}
		}
		if !matched {
			fmt.Printf("Error: Query `%v` does not match any cases\n", query)
			return 1
		}
	}

	if list {
		for _, testCase := range cases {
			fmt.Println(testCase)
		}
		return 0
	}
	return runFakeCases(cases)
}

// runFakeCases simulates running cases and returns the exit code.
func runFakeCases(cases []string) int {
	for _, testCase := range cases {
		switch {
		case strings.Contains(testCase, "crash"):
			fmt.Fprintln(os.Stderr, "[ERROR] crashing in", testCase)
			killSelf()
		case strings.Contains(testCase, "stall"):
			return 0
		case strings.Contains(testCase, "hang"):
			time.Sleep(time.Hour)
		}
		fmt.Printf("[pass] %v (1.5ms). Log:\n", testCase)
	}
	if slices.ContainsFunc(cases, isNoExit) {
		time.Sleep(time.Hour)
	}
	return 0
}

// isNoExit returns true for a test case after which the fake runner does
// not exit.
func isNoExit(testCase string) bool {
	return strings.Contains(testCase, "noexit")
}

// nonEmptyLines returns the lines of text that are not empty.
func nonEmptyLines(text string) []string {
	lines := []string{}
	for _, line := range strings.Split(text, "\n") {
		if line = strings.TrimRight(line, "\r"); line != "" {
			lines = append(lines, line)
		}
	}
	return lines
}

// killSelf ends the process abruptly, as a native crash would.
func killSelf() {
	if p, err := os.FindProcess(os.Getpid()); err == nil {
		p.Kill()
	}
	time.Sleep(time.Hour)
}

// newFakeCTS returns a CTS directory in which the fake runner finds cases.
func newFakeCTS(t *testing.T, cases []string) string {
	t.Helper()
	dir := t.TempDir()
	if err := os.MkdirAll(filepath.Join(dir, "out-node"), 0777); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "out-node", "cases.txt"), []byte(joinLines(cases)), 0666); err != nil {
		t.Fatal(err)
	}
	return dir
}
