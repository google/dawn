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

package common

import (
	"fmt"
	"maps"
	"regexp"
	"slices"
	"strings"
	"unicode"

	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/reducedglob"
)

// expectationsFile holds what a file in the format of
// webgpu-cts/expectations.txt, or a results file written by -output, expects
// of the test cases of a run. The query that names a test case decides for it,
// else the longest wildcard query that matches it, and among those of the same
// length the first one in the file.
type expectationsFile struct {
	path            string
	fromResultsFile bool                           // the file is a results file written by -output: its lines have no tags and no line numbers
	lines           []expectationLine              // every line of the file
	declaredTags    result.Tags                    // the tags that the tag header declares, in lower case
	runTags         result.Tags                    // the tags of the run, which select the lines that apply
	exact           map[TestCase]*queryExpectation // the queries without wildcards
	wildcards       []*queryExpectation            // sorted by query length (longest first), then by their position in the file.
}

// queryExpectation is what the lines with one query that apply to the run
// expect of the test cases that the query matches.
type queryExpectation struct {
	matcher  *expectations.Expectation // the first line with the query, which caches its glob matcher
	results  expectedResults           // union of the results of the lines that apply, 0 if none does
	skipLine int                       // number of the first line with Skip that applies, 0 if none does
}

// expectationLine is a parsed line of an expectations file with its tags in
// lower case and its results as a set.
type expectationLine struct {
	parsed  *expectations.Expectation
	tags    result.Tags
	results expectedResults
}

// expectedResults is a set of the results that a line can expect.
type expectedResults uint8

const (
	expectSkip           expectedResults = 1 << iota // do not run
	expectFailure                                    // fails, crashes or times out
	expectRetryOnFailure                             // may have any status
	expectSlow                                       // may time out
)

// resultOfName maps the name of a result in an expectations file to the result.
var resultOfName = map[string]expectedResults{
	"Skip":           expectSkip,
	"Failure":        expectFailure,
	"RetryOnFailure": expectRetryOnFailure,
	"Slow":           expectSlow,
}

// resultsFileStart matches the start of a results file written by -output, a
// JSON array of objects.
var resultsFileStart = regexp.MustCompile(`^\s*\[\s*[{\]]`)

// isResultsFile returns true if text is empty or starts like a results file.
// An expectations.txt file starts like one only if its first line has an empty
// list of tags.
func isResultsFile(text []byte) bool {
	return len(text) == 0 || resultsFileStart.Match(text)
}

// loadExpectations loads the file at path, which is either a results file
// written by -output or in the format of webgpu-cts/expectations.txt.
// commaSeparatedTags are the tags of the run, which select the lines of the
// latter. A results file has no tags, so all of it applies.
func (s *State) loadExpectations(path, commaSeparatedTags string, fsReader oswrapper.FilesystemReader) error {
	text, err := fsReader.ReadFile(path)
	if err != nil {
		return fmt.Errorf("failed to open expectations file: %w", err)
	}
	if isResultsFile(text) {
		s.expectFile, err = parseResultsFile(path, text)
		return err
	}
	s.expectFile, err = parseExpectationsFile(path, string(text), parseRunTags(commaSeparatedTags))
	return err
}

// parseResultsFile parses text, the contents of the results file at path, as
// expectations: each of its test cases is a line without tags, except for those
// that it records as passed, which expect nothing.
func parseResultsFile(path string, text []byte) (*expectationsFile, error) {
	statuses := Expectations{}
	if err := statuses.parse(text); err != nil {
		return nil, err
	}
	expectFile := &expectationsFile{path: path, fromResultsFile: true}
	for testCase, status := range statuses {
		results := resultsOfStatus(status)
		if results == 0 {
			continue
		}
		parsed := &expectations.Expectation{Query: queryOfTestCase(testCase)}
		expectFile.lines = append(expectFile.lines, expectationLine{parsed: parsed, results: results})
	}
	expectFile.selectLines(result.NewTags())
	return expectFile, nil
}

// resultsOfStatus returns what a results file that records status for a test
// case expects of it: a skipped one is skipped, and one that failed, crashed or
// timed out fails.
func resultsOfStatus(status Status) expectedResults {
	switch {
	case status == Skip:
		return expectSkip
	case status.failedOrTimedOut():
		return expectFailure
	}
	return 0
}

// parseRunTags returns the tags in commaSeparatedTags, in lower case. Spaces
// around the tags and empty tags are ignored.
func parseRunTags(commaSeparatedTags string) result.Tags {
	return result.NewTags(strings.FieldsFunc(strings.ToLower(commaSeparatedTags), isTagSeparator)...)
}

// isTagSeparator returns true if char is a comma or a space.
func isTagSeparator(char rune) bool {
	return char == ',' || unicode.IsSpace(char)
}

// parseExpectationsFile parses text, the content of the expectations file at
// path, for a run with runTags. Tags are compared in lower case.
func parseExpectationsFile(path, text string, runTags result.Tags) (*expectationsFile, error) {
	fileContent, err := expectations.Parse(path, text)
	if err != nil {
		return nil, err
	}
	lines, err := readLines(path, fileContent)
	if err != nil {
		return nil, err
	}
	expectFile := &expectationsFile{path: path, lines: lines, declaredTags: declaredTags(fileContent)}
	if err := expectFile.checkRunTagsDeclared(runTags); err != nil {
		return nil, err
	}
	if err := expectFile.checkLineTagsDeclared(); err != nil {
		return nil, err
	}
	expectFile.selectLines(runTags)
	return expectFile, nil
}

// readLines returns the expectation lines of fileContent, the parsed file at
// path, with their tags in lower case and their results.
func readLines(path string, fileContent expectations.Content) ([]expectationLine, error) {
	lines := []expectationLine{}
	for _, chunk := range fileContent.Chunks {
		for i := range chunk.Expectations {
			parsed := &chunk.Expectations[i]
			results, err := parseResults(parsed.Status)
			if err != nil {
				return nil, fmt.Errorf("%v:%v: %w", path, parsed.Line, err)
			}
			lines = append(lines, expectationLine{parsed: parsed, tags: lowerTags(parsed.Tags), results: results})
		}
	}
	return lines, nil
}

// parseResults returns the set of results that names lists. It is an error for
// names to be empty or to have an unknown name.
func parseResults(names []string) (expectedResults, error) {
	if len(names) == 0 {
		return 0, fmt.Errorf("no results")
	}
	results := expectedResults(0)
	for _, name := range names {
		expected, ok := resultOfName[name]
		if !ok {
			known := strings.Join(slices.Sorted(maps.Keys(resultOfName)), ", ")
			return 0, fmt.Errorf("unknown result '%v', the results are %v", name, known)
		}
		results |= expected
	}
	return results, nil
}

// lowerTags returns tags in lower case.
func lowerTags(tags result.Tags) result.Tags {
	lower := result.NewTags()
	for tag := range tags {
		lower.Add(strings.ToLower(tag))
	}
	return lower
}

// declaredTags returns the tags that the header of the file declares, in lower
// case. It is empty for a file without a header.
func declaredTags(fileContent expectations.Content) result.Tags {
	declared := result.NewTags()
	for _, tagSet := range fileContent.Tags.Sets {
		declared.AddAll(lowerTags(tagSet.Tags))
	}
	return declared
}

// declares returns true if the tag header declares tag. A file without a tag
// header declares every tag.
func (f *expectationsFile) declares(tag string) bool {
	return len(f.declaredTags) == 0 || f.declaredTags.Contains(tag)
}

// checkRunTagsDeclared returns an error if the file does not declare one of
// runTags.
func (f *expectationsFile) checkRunTagsDeclared(runTags result.Tags) error {
	if tag := f.findUndeclaredTag(runTags); tag != "" {
		return fmt.Errorf("-expect-tags: %v does not declare the tag '%v'", f.path, tag)
	}
	return nil
}

// checkLineTagsDeclared returns an error if the file does not declare a tag of
// one of its lines.
func (f *expectationsFile) checkLineTagsDeclared() error {
	for _, line := range f.lines {
		if tag := f.findUndeclaredTag(line.tags); tag != "" {
			return fmt.Errorf("%v:%v: the tag header does not declare the tag '%v'", f.path, line.parsed.Line, tag)
		}
	}
	return nil
}

// findUndeclaredTag returns one of tags that the file does not declare, or ""
// if there is none.
func (f *expectationsFile) findUndeclaredTag(tags result.Tags) string {
	for _, tag := range tags.List() {
		if !f.declares(tag) {
			return tag
		}
	}
	return ""
}

// groupByQuery returns an expectation for each query of lines, in the order of
// the first lines of the queries, that combines the lines of the query whose
// tags are all in runTags.
func groupByQuery(lines []expectationLine, runTags result.Tags) []*queryExpectation {
	queryExpectations := []*queryExpectation{}
	expectationOfQuery := map[string]*queryExpectation{}
	for _, line := range lines {
		expectation := expectationOfQuery[line.parsed.Query]
		if expectation == nil {
			expectation = &queryExpectation{matcher: line.parsed}
			expectationOfQuery[line.parsed.Query] = expectation
			queryExpectations = append(queryExpectations, expectation)
		}
		if runTags.ContainsAll(line.tags) {
			expectation.add(line)
		}
	}
	return queryExpectations
}

// add combines line, which applies to the run, into the expectation.
func (e *queryExpectation) add(line expectationLine) {
	e.results |= line.results
	if e.skipLine == 0 && line.results.has(expectSkip) {
		e.skipLine = line.parsed.Line
	}
}

// selectLines sets the tags of the run to runTags and rebuilds exact and
// wildcards from the lines whose tags are all in runTags. A query without such
// a line is left out.
func (f *expectationsFile) selectLines(runTags result.Tags) {
	f.runTags = runTags
	f.exact = map[TestCase]*queryExpectation{}
	f.wildcards = nil
	for _, expectation := range groupByQuery(f.lines, runTags) {
		if expectation.results == 0 {
			continue
		}
		if expectation.matcher.IsGlobExpectation() {
			f.wildcards = append(f.wildcards, expectation)
		} else {
			f.exact[testCaseOfQuery(expectation.matcher.Query)] = expectation
		}
	}
	// The entries in f.wildcards are in the order in which they appeared
	// in the file. Sort them by query length. If one query is a subset of another
	// it will be longer, and should be moved earlier. We use stable sort to
	// maintain the order in the original file when there is a tie.
	slices.SortStableFunc(f.wildcards, compareQueryLength)
}

// queryOfTestCase returns the query without wildcards that names testCase, a
// name as it is written in a results JSON file. A '*' in the name is '\*' in
// the query.
func queryOfTestCase(testCase TestCase) string {
	return strings.ReplaceAll(string(testCase), reducedglob.UNESCAPED_WILDCARD, reducedglob.ESCAPED_WILDCARD)
}

// testCaseOfQuery returns the test case that query, which has no wildcards,
// names. A '\*' in a query stands for a '*' in the name.
func testCaseOfQuery(query string) TestCase {
	return TestCase(strings.ReplaceAll(query, reducedglob.ESCAPED_WILDCARD, reducedglob.UNESCAPED_WILDCARD))
}

// compareQueryLength orders the expectation with the longer query first.
func compareQueryLength(one, other *queryExpectation) int {
	return len(other.matcher.Query) - len(one.matcher.Query)
}

// lookup returns the expectation for testCase, or the zero expectation if no
// query matches it.
func (f *expectationsFile) lookup(testCase TestCase) queryExpectation {
	if expectation, ok := f.exact[testCase]; ok {
		return *expectation
	}
	for _, expectation := range f.wildcards {
		if expectation.matcher.AppliesToTest(string(testCase)) {
			return *expectation
		}
	}
	return queryExpectation{}
}

// has returns true if expected is one of the results.
func (r expectedResults) has(expected expectedResults) bool {
	return r&expected != 0
}

// displayStatus returns the status to print as the expected one of a test
// case, or "" if the results do not name one.
func (r expectedResults) displayStatus() Status {
	switch {
	case r.has(expectSkip):
		return Skip
	case r.has(expectFailure):
		return Fail
	}
	return ""
}

// unexpected returns true if status is not one that the results allow. A test
// case that the CTS skips is never unexpected.
func (r expectedResults) unexpected(status Status) bool {
	switch {
	case r.has(expectSkip), r.has(expectRetryOnFailure), status == Skip:
		return false
	case r.has(expectSlow) && status == Timeout:
		return false
	case r.has(expectFailure):
		return !status.failedOrTimedOut()
	}
	return status.failedOrTimedOut()
}

// RemoveSkipped returns the test cases that the expectations file does not
// skip, and a skip result for each of the others.
func (s *State) RemoveSkipped(testCases []TestCase) ([]TestCase, []Result) {
	remaining := make([]TestCase, 0, len(testCases))
	var skipped []Result
	for _, testCase := range testCases {
		expectation := s.expectation(testCase)
		if !expectation.results.has(expectSkip) {
			remaining = append(remaining, testCase)
			continue
		}
		skipped = append(skipped, Result{
			TestCase: testCase,
			Status:   Skip,
			Message:  fmt.Sprintf("skipped by %v\n", s.expectFile.skippedBy(expectation)),
		})
	}
	// The runners give the results of the remaining test cases the indices from 0.
	for i := range skipped {
		skipped[i].Index = len(remaining) + i
	}
	return remaining, skipped
}

// skippedBy returns where the file skips the test cases of expectation: the
// line with Skip, or just the file for a results file, which has no line
// numbers.
func (f *expectationsFile) skippedBy(expectation queryExpectation) string {
	if f.fromResultsFile {
		return f.path
	}
	return fmt.Sprintf("%v:%v", f.path, expectation.skipLine)
}

// HasExpectationsFile returns true if -expect names a file.
func (s *State) HasExpectationsFile() bool {
	return s.expectFile != nil
}

// HasTaggedExpectationsFile returns true if -expect names a file in the format
// of webgpu-cts/expectations.txt, whose lines the tags of the run select. A
// results file has no tags.
func (s *State) HasTaggedExpectationsFile() bool {
	return s.expectFile != nil && !s.expectFile.fromResultsFile
}

// expectation returns what the -expect file expects of testCase. Without one
// it is the zero expectation, which expects a test case not to fail.
func (s *State) expectation(testCase TestCase) queryExpectation {
	if !s.HasExpectationsFile() {
		return queryExpectation{}
	}
	return s.expectFile.lookup(testCase)
}

// resultsFileStatus returns the status that -output records for testCase,
// which had status: XFail for a failure that is expected, else status.
func (s *State) resultsFileStatus(testCase TestCase, status Status) Status {
	if status.failedOrTimedOut() && !s.expectation(testCase).results.unexpected(status) {
		return XFail
	}
	return status
}

// AddAutomaticTags adds automaticTags to the tags of the run, except for those
// that the expectations file does not declare, and selects the lines of the
// file again. It returns the tags of the run.
func (s *State) AddAutomaticTags(automaticTags []string) result.Tags {
	runTags := s.expectFile.runTags.Clone()
	for _, tag := range automaticTags {
		tag = strings.ToLower(tag)
		if s.expectFile.declares(tag) {
			runTags.Add(tag)
		}
	}
	s.expectFile.selectLines(runTags)
	return runTags
}
