// Copyright 2021 The Tint Authors.
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
	switch err := exec.Command(exe, "--gtest_output=json:"+testResultsPath).Run().(type) {
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
	var errs []error
	numFixes := 0
	for _, group := range testResults.Groups {
		for _, suite := range group.Testsuites {
			for _, failure := range suite.Failures {
				// .. attempt to fix the problem
				test := group.Name + "." + suite.Name
				if err := processFailure(test, wd, failure.Failure); err != nil {
					errs = append(errs, fmt.Errorf("%v: %w", test, err))
				} else {
					numFixes++
				}
			}
		}
	}

	if numFixes > 0 {
		fmt.Printf("%v tests fixed\n", numFixes)
	}
	if n := len(errs); n > 0 {
		fmt.Printf("%v tests could not be fixed:\n", n)
		for _, err := range errs {
			fmt.Println(err)
		}
	}
	return nil
}

var (
	// Regular expression to match a test declaration
	reTests = regexp.MustCompile(`TEST(?:_[FP])?\((\w+),[ \n]*(\w+)\)`)
	// Regular expression to match a EXPECT_EQ failure for strings
	reExpectEq = regexp.MustCompile(`^([./\\a-z_-]*):(\d+).*\nExpected equality of these values:\n(?:.|\n)*?(?:Which is: |  )"((?:.|\n)*?)[^\\]"\n(?:.|\n)*?(?:Which is: |  )"((?:.|\n)*?)[^\\]"`)
)

func processFailure(test, wd, failure string) error {
	// Start by un-escaping newlines in the failure message
	failure = strings.ReplaceAll(failure, "\\n", "\n")

	// Look for a EXPECT_EQ failure pattern
	var file, a, b string
	if parts := reExpectEq.FindStringSubmatch(failure); len(parts) == 5 {
		file, a, b = parts[1], parts[3], parts[4]
	} else {
		return fmt.Errorf("Cannot fix this type of failure")
	}

	// Now un-escape any quotes (the regex is sensitive to these)
	a = strings.ReplaceAll(a, `\"`, `"`)
	b = strings.ReplaceAll(b, `\"`, `"`)

	// Get the path to the source file containing the test failure
	sourcePath := filepath.Join(wd, file)

	// Parse the source file, split into tests
	sourceFile, err := parseSourceFile(sourcePath)
	if err != nil {
		return fmt.Errorf("Couldn't parse tests from file '%v': %w", file, err)
	}

	// Find the test
	testIdx, ok := sourceFile.tests[test]
	if !ok {
		return fmt.Errorf("Test '%v' not found in '%v'", test, file)
	}

	// Grab the source for the particular test
	testSource := sourceFile.parts[testIdx]

	// We don't know if a or b is the expected, so just try flipping the string
	// to the other form.
	switch {
	case strings.Contains(testSource, a):
		testSource = strings.Replace(testSource, a, b, -1)
	case strings.Contains(testSource, b):
		testSource = strings.Replace(testSource, b, a, -1)
	default:
		// Try escaping for R"(...)" strings
		a = strings.ReplaceAll(a, "\n", `\n`)
		b = strings.ReplaceAll(b, "\n", `\n`)
		a = strings.ReplaceAll(a, "\"", `\"`)
		b = strings.ReplaceAll(b, "\"", `\"`)
		switch {
		case strings.Contains(testSource, a):
			testSource = strings.Replace(testSource, a, b, -1)
		case strings.Contains(testSource, b):
			testSource = strings.Replace(testSource, b, a, -1)
		default:
			return fmt.Errorf("Could not fix test '%v' in '%v'", test, file)
		}
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
