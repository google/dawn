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

// test-runner runs tint against a number of test shaders checking for expected behavior
package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"sync"
	"unicode/utf8"

	"dawn.googlesource.com/tint/tools/src/fileutils"
	"dawn.googlesource.com/tint/tools/src/glob"
	"github.com/fatih/color"
	"github.com/sergi/go-diff/diffmatchpatch"
)

type outputFormat string

const (
	wgsl   = outputFormat("wgsl")
	spvasm = outputFormat("spvasm")
	msl    = outputFormat("msl")
	hlsl   = outputFormat("hlsl")
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
test-runner runs tint against a number of test shaders checking for expected behavior

usage:
  test-runner [flags...] <executable> [<directory>]

  <executable> the path to the tint executable
  <directory>  the root directory of the test files

optional flags:`)
	flag.PrintDefaults()
	fmt.Println(``)
	os.Exit(1)
}

func run() error {
	var formatList, filter string
	numCPU := runtime.NumCPU()
	generateExpected := false
	flag.StringVar(&formatList, "format", "all", "comma separated list of formats to emit. Possible values are: all, wgsl, spvasm, msl, hlsl")
	flag.StringVar(&filter, "filter", "**.wgsl, **.spvasm, **.spv", "comma separated list of glob patterns for test files")
	flag.BoolVar(&generateExpected, "generate-expected", false, "create or update all expected outputs")
	flag.IntVar(&numCPU, "j", numCPU, "maximum number of concurrent threads to run tests")
	flag.Usage = showUsage
	flag.Parse()

	args := flag.Args()
	if len(args) == 0 {
		showUsage()
	}

	// executable path is the first argument
	exe, args := args[0], args[1:]

	// (optional) target directory is the second argument
	dir := "."
	if len(args) > 0 {
		dir, args = args[0], args[1:]
	}

	// Check the executable can be found and actually is executable
	if !fileutils.IsExe(exe) {
		return fmt.Errorf("'%s' not found or is not executable", exe)
	}

	// Split the --filter flag up by ',', trimming any whitespace at the start and end
	globIncludes := strings.Split(filter, ",")
	for i, s := range globIncludes {
		globIncludes[i] = `"` + strings.TrimSpace(s) + `"`
	}

	// Glob the files to test
	files, err := glob.Scan(dir, glob.MustParseConfig(`{
		"paths": [
			{
				"include": [ `+strings.Join(globIncludes, ",")+` ]
			},
			{
				"exclude": [
					"**.expected.wgsl",
					"**.expected.spvasm",
					"**.expected.msl",
					"**.expected.hlsl"
				]
			}
		]
	}`))
	if err != nil {
		return fmt.Errorf("Failed to glob files: %w", err)
	}

	// Ensure the files are sorted (globbing should do this, but why not)
	sort.Strings(files)

	// Parse --format into a list of outputFormat
	formats := []outputFormat{}
	if formatList == "all" {
		formats = []outputFormat{wgsl, spvasm, msl, hlsl}
	} else {
		for _, f := range strings.Split(formatList, ",") {
			switch strings.TrimSpace(f) {
			case "wgsl":
				formats = append(formats, wgsl)
			case "spvasm":
				formats = append(formats, spvasm)
			case "msl":
				formats = append(formats, msl)
			case "hlsl":
				formats = append(formats, hlsl)
			default:
				return fmt.Errorf("unknown format '%s'", f)
			}
		}
	}

	results := make([]map[outputFormat]*status, len(files))
	jobs := make(chan job, 256)

	// Spawn numCPU job runners...
	wg := sync.WaitGroup{}
	wg.Add(numCPU)
	for cpu := 0; cpu < numCPU; cpu++ {
		go func() {
			defer wg.Done()
			for job := range jobs {
				job.run(exe, generateExpected)
			}
		}()
	}

	// Issue the jobs...
	for i, file := range files { // For each test file...
		file := filepath.Join(dir, file)
		fileResults := map[outputFormat]*status{}
		for _, format := range formats { // For each output format...
			result := &status{}
			jobs <- job{
				file:   file,
				format: format,
				result: result,
			}
			fileResults[format] = result
		}
		results[i] = fileResults
	}

	// Wait for the jobs to all finish...
	close(jobs)
	wg.Wait()

	// Time to print the outputs

	// Start by printing the error message for any file x format combinations
	// that failed...
	for i, file := range files {
		results := results[i]
		for _, format := range formats {
			if err := results[format].err; err != nil {
				color.Set(color.FgBlue)
				fmt.Printf("%s ", file)
				color.Set(color.FgCyan)
				fmt.Printf("%s ", format)
				color.Set(color.FgRed)
				fmt.Println("FAIL")
				color.Unset()
				fmt.Println(indent(err.Error(), 4))
			}
		}
	}

	// Now print the table of file x format
	numTests, numPass, numSkip, numFail := 0, 0, 0, 0
	filenameFmt := columnFormat(maxStringLen(files), false)

	fmt.Println()
	fmt.Printf(filenameFmt, "")
	fmt.Printf(" ┃ ")
	for _, format := range formats {
		color.Set(color.FgCyan)
		fmt.Printf(columnFormat(formatWidth(format), false), format)
		color.Unset()
		fmt.Printf(" │ ")
	}
	fmt.Println()
	fmt.Printf(strings.Repeat("━", maxStringLen(files)))
	fmt.Printf("━╋━")
	for _, format := range formats {
		fmt.Printf(strings.Repeat("━", formatWidth(format)))
		fmt.Printf("━│━")
	}
	fmt.Println()

	for i, file := range files {
		results := results[i]

		color.Set(color.FgBlue)
		fmt.Printf(filenameFmt, file)
		color.Unset()
		fmt.Printf(" ┃ ")
		for _, format := range formats {
			formatFmt := columnFormat(formatWidth(format), true)
			result := results[format]
			numTests++
			switch result.code {
			case pass:
				color.Set(color.FgGreen)
				fmt.Printf(formatFmt, "PASS")
				numPass++
			case fail:
				color.Set(color.FgRed)
				fmt.Printf(formatFmt, "FAIL")
				numFail++
			case skip:
				color.Set(color.FgYellow)
				fmt.Printf(formatFmt, "SKIP")
				numSkip++
			default:
				fmt.Printf(formatFmt, result.code)
			}
			color.Unset()
			fmt.Printf(" │ ")
		}
		fmt.Println()
	}
	fmt.Println()

	fmt.Printf("%d tests run", numTests)
	if numPass > 0 {
		fmt.Printf(", ")
		color.Set(color.FgGreen)
		fmt.Printf("%d tests pass", numPass)
		color.Unset()
	} else {
		fmt.Printf(", %d tests pass", numPass)
	}
	if numSkip > 0 {
		fmt.Printf(", ")
		color.Set(color.FgYellow)
		fmt.Printf("%d tests skipped", numSkip)
		color.Unset()
	} else {
		fmt.Printf(", %d tests skipped", numSkip)
	}
	if numFail > 0 {
		fmt.Printf(", ")
		color.Set(color.FgRed)
		fmt.Printf("%d tests failed", numFail)
		color.Unset()
	} else {
		fmt.Printf(", %d tests failed", numFail)
	}
	fmt.Println()
	fmt.Println()

	if numFail > 0 {
		os.Exit(1)
	}

	return nil
}

// Structures to hold the results of the tests
type statusCode string

const (
	fail statusCode = "FAIL"
	pass statusCode = "PASS"
	skip statusCode = "SKIP"
)

type status struct {
	code statusCode
	err  error
}

type job struct {
	file   string
	format outputFormat
	result *status
}

func (j job) run(exe string, generateExpected bool) {
	// Is there an expected output?
	expected := loadExpectedFile(j.file, j.format)
	if strings.HasPrefix(expected, "SKIP") { // Special SKIP token
		*j.result = status{code: skip}
		return
	}

	// Invoke the compiler...
	var err error
	if ok, out := invoke(exe, j.file, "--format", string(j.format), "--dawn-validation"); ok {
		if generateExpected {
			// If --generate-expected was passed, write out the output
			err = saveExpectedFile(j.file, j.format, out)
		} else if expected != "" && expected != out {
			// Expected output did not match
			dmp := diffmatchpatch.New()
			diff := dmp.DiffPrettyText(dmp.DiffMain(expected, out, true))
			err = fmt.Errorf(`Output was not as expected

--------------------------------------------------------------------------------
-- Expected:                                                                  --
--------------------------------------------------------------------------------
%s

--------------------------------------------------------------------------------
-- Got:                                                                       --
--------------------------------------------------------------------------------
%s

--------------------------------------------------------------------------------
-- Diff:                                                                      --
--------------------------------------------------------------------------------
%s`,
				expected, out, diff)
		}
	} else {
		// Compiler returned a non-zero exit code
		err = fmt.Errorf("%s", out)
	}

	if err != nil {
		*j.result = status{code: fail, err: err}
	} else {
		*j.result = status{code: pass}
	}
}

// loadExpectedFile loads the expected output file for the test file at 'path'
// and the output format 'format'. If the file does not exist, or cannot be
// read, then an empty string is returned.
func loadExpectedFile(path string, format outputFormat) string {
	content, err := ioutil.ReadFile(expectedFilePath(path, format))
	if err != nil {
		return ""
	}
	return string(content)
}

// saveExpectedFile writes the expected output file for the test file at 'path'
// and the output format 'format', with the content 'content'.
func saveExpectedFile(path string, format outputFormat, content string) error {
	return ioutil.WriteFile(expectedFilePath(path, format), []byte(content), 0666)
}

// expectedFilePath returns the expected output file path for the test file at
// 'path' and the output format 'format'.
func expectedFilePath(path string, format outputFormat) string {
	return path + ".expected." + string(format)
}

// indent returns the string 's' indented with 'n' whitespace characters
func indent(s string, n int) string {
	tab := strings.Repeat(" ", n)
	return tab + strings.ReplaceAll(s, "\n", "\n"+tab)
}

// columnFormat returns the printf format string to sprint a string with the
// width of 'i' runes.
func columnFormat(i int, alignLeft bool) string {
	if alignLeft {
		return "%-" + strconv.Itoa(i) + "s"
	}
	return "%" + strconv.Itoa(i) + "s"
}

// maxStringLen returns the maximum number of runes found in all the strings in
// 'l'
func maxStringLen(l []string) int {
	max := 0
	for _, s := range l {
		if c := utf8.RuneCountInString(s); c > max {
			max = c
		}
	}
	return max
}

// formatWidth returns the width in runes for the outputFormat column 'b'
func formatWidth(b outputFormat) int {
	c := utf8.RuneCountInString(string(b))
	if c > 4 {
		return c
	}
	return 4
}

// invoke runs the executable 'exe' with the provided arguments.
func invoke(exe string, args ...string) (ok bool, output string) {
	cmd := exec.Command(exe, args...)
	out, err := cmd.CombinedOutput()
	str := string(out)
	if err != nil {
		if str != "" {
			return false, str
		}
		return false, err.Error()
	}
	return true, str
}
