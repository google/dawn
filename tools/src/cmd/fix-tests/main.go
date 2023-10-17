// Copyright 2021 The Dawn & Tint Authors
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

// fix-tests is a tool to update tests with new expected output.
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/substr"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
fix-tests is a tool to update tests with new expected output.

fix-tests performs string matching and heuristics to fix up expected results of
tests that use EXPECT_EQ(a, b) and EXPECT_THAT(a, HasSubstr(b))

WARNING: Always thoroughly check the generated output for mistakes.
This may produce incorrect output

Usage:
  fix-tests <executable>

  executable         - the path to the test executable to run.`)
	os.Exit(1)
}

func run() error {
	flag.Parse()
	args := flag.Args()
	if len(args) < 1 {
		showUsage()
	}

	exe := args[0]          // The path to the test executable
	wd := filepath.Dir(exe) // The directory holding the test exe

	// Create a temporary directory to hold the 'test-results.json' file
	tmpDir, err := ioutil.TempDir("", "fix-tests")
	if err != nil {
		return err
	}
	if err := os.MkdirAll(tmpDir, 0666); err != nil {
		return fmt.Errorf("Failed to create temporary directory: %w", err)
	}
	defer os.RemoveAll(tmpDir)

	// Full path to the 'test-results.json' in the temporary directory
	testResultsPath := filepath.Join(tmpDir, "test-results.json")

	// Run the tests
	testArgs := []string{"--gtest_output=json:" + testResultsPath}
	if len(args) > 1 {
		testArgs = append(testArgs, args[1:]...)
	}
	switch err := exec.Command(exe, testArgs...).Run().(type) {
	default:
		return err
	case nil:
		fmt.Println("All tests passed")
	case *exec.ExitError:
	}

	// Read the 'test-results.json' file
	testResultsFile, err := os.Open(testResultsPath)
	if err != nil {
		return err
	}

	var testResults Results
	if err := json.NewDecoder(testResultsFile).Decode(&testResults); err != nil {
		return err
	}

	// For each failing test...
	seen := map[string]bool{}
	numFixed, numFailed := 0, 0
	for _, group := range testResults.Groups {
		for _, suite := range group.Testsuites {
			for _, failure := range suite.Failures {
				// .. attempt to fix the problem
				test := testName(group, suite)
				if seen[test] {
					continue
				}
				seen[test] = true

				if err := processFailure(test, wd, failure.Failure); err != nil {
					fmt.Println(fmt.Errorf("%v: %w", test, err))
					numFailed++
				} else {
					numFixed++
				}
			}
		}
	}

	fmt.Println()

	if numFailed > 0 {
		fmt.Println(numFailed, "tests could not be fixed")
	}
	if numFixed > 0 {
		fmt.Println(numFixed, "tests fixed")
	}
	return nil
}

func testName(group TestsuiteGroup, suite Testsuite) string {
	groupParts := strings.Split(group.Name, "/")
	suiteParts := strings.Split(suite.Name, "/")
	return groupParts[len(groupParts)-1] + "." + suiteParts[0]
}

var (
	// Regular expression to match a test declaration
	reTests = regexp.MustCompile(`TEST(?:_[FP])?\([ \n]*(\w+),[ \n]*(\w+)\)`)
	// Regular expression to match a `EXPECT_EQ(a, b)` failure for strings
	reExpectEq = regexp.MustCompile(`([./\\\w_\-:]*):(\d+).*\nExpected equality of these values:\n(?:.|\n)*?(?:Which is: |  )"((?:.|\n)*?[^\\])"\n(?:.|\n)*?(?:Which is: |  )"((?:.|\n)*?[^\\])"`)
	// Regular expression to match a `EXPECT_THAT(a, HasSubstr(b))` failure for strings
	reExpectHasSubstr = regexp.MustCompile(`([./\\\w_\-:]*):(\d+).*\nValue of: .*\nExpected: has substring "((?:.|\n)*?[^\\])"\n  Actual: "((?:.|\n)*?[^\\])"`)
)

func processFailure(test, wd, failure string) error {
	// Start by un-escaping newlines in the failure message
	failure = strings.ReplaceAll(failure, "\\n", "\n")
	// Matched regex strings will also need to be un-escaped, but do this after
	// the match, as unescaped quotes may upset the regex patterns
	unescape := func(s string) string {
		return strings.ReplaceAll(s, `\"`, `"`)
	}
	escape := func(s string) string {
		s = strings.ReplaceAll(s, "\n", `\n`)
		s = strings.ReplaceAll(s, "\"", `\"`)
		return s
	}

	// Look for a EXPECT_EQ failure pattern
	var file string
	var fix func(testSource string) (string, error)
	if parts := reExpectEq.FindStringSubmatch(failure); len(parts) == 5 {
		// EXPECT_EQ(a, b)
		a, b := unescape(parts[3]), unescape(parts[4])
		file = parts[1]
		fix = func(testSource string) (string, error) {
			// We don't know if a or b is the expected, so just try flipping the string
			// to the other form.

			if len(b) > len(a) { // Go with the longer match, in case both are found
				a, b = b, a
			}
			switch {
			case strings.Contains(testSource, a):
				testSource = strings.ReplaceAll(testSource, a, b)
			case strings.Contains(testSource, b):
				testSource = strings.ReplaceAll(testSource, b, a)
			default:
				// Try escaping for R"(...)" strings
				a, b = escape(a), escape(b)
				switch {
				case strings.Contains(testSource, a):
					testSource = strings.ReplaceAll(testSource, a, b)
				case strings.Contains(testSource, b):
					testSource = strings.ReplaceAll(testSource, b, a)
				default:
					return "", fmt.Errorf("Could not fix 'EXPECT_EQ' pattern in '%v'", file)
				}
			}
			return testSource, nil
		}
	} else if parts := reExpectHasSubstr.FindStringSubmatch(failure); len(parts) == 5 {
		// EXPECT_THAT(a, HasSubstr(b))
		a, b := unescape(parts[4]), unescape(parts[3])
		file = parts[1]
		fix = func(testSource string) (string, error) {
			if fix := substr.Fix(a, b); fix != "" {
				if !strings.Contains(testSource, b) {
					// Try escaping for R"(...)" strings
					b, fix = escape(b), escape(fix)
				}
				if strings.Contains(testSource, b) {
					testSource = strings.Replace(testSource, b, fix, -1)
					return testSource, nil
				}
				return "", fmt.Errorf("Could apply fix for 'HasSubstr' pattern in '%v'", file)
			}

			return "", fmt.Errorf("Could find fix for 'HasSubstr' pattern in '%v'", file)
		}
	} else {
		return fmt.Errorf("Cannot fix this type of failure")
	}

	// Get the absolute source path
	sourcePath := file
	if !filepath.IsAbs(sourcePath) {
		sourcePath = filepath.Join(wd, file)
	}

	// Parse the source file, split into tests
	sourceFile, err := parseSourceFile(sourcePath)
	if err != nil {
		return fmt.Errorf("Couldn't parse tests from file '%v': %w", file, err)
	}

	// Find the test
	testIdx, ok := sourceFile.tests[test]
	if !ok {
		return fmt.Errorf("Test not found in '%v'", file)
	}

	// Grab the source for the particular test
	testSource := sourceFile.parts[testIdx]

	if testSource, err = fix(testSource); err != nil {
		return err
	}

	// Replace the part of the source file
	sourceFile.parts[testIdx] = testSource

	// Write out the source file
	return writeSourceFile(sourcePath, sourceFile)
}

// parseSourceFile() reads the file at path, splitting the content into chunks
// for each TEST.
func parseSourceFile(path string) (sourceFile, error) {
	fileBytes, err := ioutil.ReadFile(path)
	if err != nil {
		return sourceFile{}, err
	}
	fileContent := string(fileBytes)

	out := sourceFile{
		tests: map[string]int{},
	}

	pos := 0
	for _, span := range reTests.FindAllStringIndex(fileContent, -1) {
		out.parts = append(out.parts, fileContent[pos:span[0]])
		pos = span[0]

		match := reTests.FindStringSubmatch(fileContent[span[0]:span[1]])
		group := match[1]
		suite := match[2]
		out.tests[group+"."+suite] = len(out.parts)
	}
	out.parts = append(out.parts, fileContent[pos:])

	return out, nil
}

// writeSourceFile() joins the chunks of the file, and writes the content out to
// path.
func writeSourceFile(path string, file sourceFile) error {
	body := strings.Join(file.parts, "")
	return ioutil.WriteFile(path, []byte(body), 0666)
}

type sourceFile struct {
	parts []string
	tests map[string]int // "X.Y" -> part index
}

// Results is the root JSON structure of the JSON --gtest_output file .
type Results struct {
	Tests     int              `json:"tests"`
	Failures  int              `json:"failures"`
	Disabled  int              `json:"disabled"`
	Errors    int              `json:"errors"`
	Timestamp string           `json:"timestamp"`
	Time      string           `json:"time"`
	Name      string           `json:"name"`
	Groups    []TestsuiteGroup `json:"testsuites"`
}

// TestsuiteGroup is a group of test suites in the JSON --gtest_output file .
type TestsuiteGroup struct {
	Name       string      `json:"name"`
	Tests      int         `json:"tests"`
	Failures   int         `json:"failures"`
	Disabled   int         `json:"disabled"`
	Errors     int         `json:"errors"`
	Timestamp  string      `json:"timestamp"`
	Time       string      `json:"time"`
	Testsuites []Testsuite `json:"testsuite"`
}

// Testsuite is a suite of tests in the JSON --gtest_output file.
type Testsuite struct {
	Name       string    `json:"name"`
	ValueParam string    `json:"value_param,omitempty"`
	Status     Status    `json:"status"`
	Result     Result    `json:"result"`
	Timestamp  string    `json:"timestamp"`
	Time       string    `json:"time"`
	Classname  string    `json:"classname"`
	Failures   []Failure `json:"failures,omitempty"`
}

// Failure is a reported test failure in the JSON --gtest_output file.
type Failure struct {
	Failure string `json:"failure"`
	Type    string `json:"type"`
}

// Status is a status code in the JSON --gtest_output file.
type Status string

// Result is a result code in the JSON --gtest_output file.
type Result string
