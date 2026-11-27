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
	"bytes"
	"context"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"github.com/stretchr/testify/require"
)

const tagHeader = `# BEGIN TAG HEADER
# OS
# tags: [ linux win ]
# GPU
# tags: [ intel nvidia ]
# END TAG HEADER
`

// parseLines parses an expectations file made of lines for a run with tags.
func parseLines(t *testing.T, tags []string, lines ...string) *expectationsFile {
	t.Helper()
	expectFile, err := parseExpectationsFile("expectations.txt", strings.Join(lines, "\n"), result.NewTags(tags...))
	require.NoError(t, err)
	return expectFile
}

// Spaces around the tags of -expect-tags and empty tags are ignored.
func TestParseRunTags(t *testing.T) {
	require.Equal(t, result.NewTags("linux", "intel"), parseRunTags(" Linux, intel,,"))
	require.Empty(t, parseRunTags(""))
}

// A results file is told from an expectations.txt file by how it starts, so
// that a damaged one is still read as a results file.
func TestIsResultsFile(t *testing.T) {
	for text, expected := range map[string]bool{
		"[\n  {\n    \"TestCase\": \"suite:a:\",": true,
		"[]\n":                          true,
		"":                              true,
		"[ linux ] suite:a:* [ Skip ]":  false,
		"# OS\n# tags: [ linux win ]\n": false,
		"suite:a:* [ Skip ]":            false,
	} {
		require.Equal(t, expected, isResultsFile([]byte(text)), text)
	}
}

// A line applies when the run has all of its tags.
func TestExpectationTags(t *testing.T) {
	lines := []string{
		tagHeader,
		"[ linux ] suite:os:* [ Failure ]",
		"[ linux intel ] suite:os_gpu:* [ Failure ]",
		"[ Linux ] suite:upper_case:* [ Failure ]",
		"suite:untagged:* [ Failure ]",
	}
	for _, test := range []struct {
		tags     []string
		testCase TestCase
		applies  bool
	}{
		{[]string{"linux"}, "suite:os:", true},
		{[]string{"linux", "nvidia"}, "suite:os:", true},
		{[]string{"win"}, "suite:os:", false},
		{nil, "suite:os:", false},
		{[]string{"linux", "intel"}, "suite:os_gpu:", true},
		{[]string{"linux"}, "suite:os_gpu:", false},
		{[]string{"intel"}, "suite:os_gpu:", false},
		{[]string{"linux"}, "suite:upper_case:", true},
		{nil, "suite:untagged:", true},
		{[]string{"win", "nvidia"}, "suite:untagged:", true},
	} {
		expectFile := parseLines(t, test.tags, lines...)
		applies := expectFile.lookup(test.testCase).results == expectFailure
		require.Equal(t, test.applies, applies, "%v with tags %v", test.testCase, test.tags)
	}
}

// A file with a tag header rejects tags that it does not declare.
func TestExpectationUndeclaredTags(t *testing.T) {
	_, err := parseExpectationsFile("expectations.txt", tagHeader, result.NewTags("linux", "lunix"))
	require.ErrorContains(t, err, "-expect-tags: expectations.txt does not declare the tag 'lunix'")

	_, err = parseExpectationsFile("expectations.txt", tagHeader+"\n[ lunix ] suite:a:* [ Skip ]", result.NewTags())
	require.ErrorContains(t, err, "expectations.txt:8: the tag header does not declare the tag 'lunix'")

	expectFile := parseLines(t, []string{"lunix"}, "[ lunix ] suite:a:* [ Skip ]")
	require.Equal(t, expectSkip, expectFile.lookup("suite:a:").results)
}

// An automatic tag that the file does not declare is left out of the run's
// tags, and one of -expect-tags is an error.
func TestAddAutomaticTags(t *testing.T) {
	lines := tagHeader + "[ linux intel ] suite:a:* [ Failure ]"
	state := &State{expectFile: parseLines(t, []string{"intel"}, lines)}

	tags := state.AddAutomaticTags([]string{"Linux", "ubuntu", "intel-0x9bc5"})
	require.Equal(t, result.NewTags("intel", "linux"), tags)
	require.Equal(t, expectFailure, state.expectFile.lookup("suite:a:").results)

	_, err := parseExpectationsFile("expectations.txt", lines, result.NewTags("ubuntu"))
	require.ErrorContains(t, err, "-expect-tags: expectations.txt does not declare the tag 'ubuntu'")
}

// Only Failure, RetryOnFailure, Skip and Slow are results, and a line needs
// one.
func TestExpectationResults(t *testing.T) {
	_, err := parseExpectationsFile("expectations.txt", "suite:a:* [ Skip ]\nsuite:b:* [ Pass ]", result.NewTags())
	require.ErrorContains(t, err, "expectations.txt:2: unknown result 'Pass', the results are Failure, RetryOnFailure, Skip, Slow")

	_, err = parseExpectationsFile("expectations.txt", "suite:a:* [ Skip ]\nsuite:a:b=1;* [ ]", result.NewTags())
	require.ErrorContains(t, err, "expectations.txt:2: no results")
}

// A '*' matches any characters anywhere in a query, and '\*' matches a '*'.
func TestExpectationWildcards(t *testing.T) {
	expectFile := parseLines(t, nil,
		"suite:end:* [ Failure ]",
		`suite:middle:*;b=2;* [ Failure ]`,
		`suite:escaped:op="\*" [ Failure ]`,
		`suite:escaped_glob:op="\*";* [ Failure ]`,
		"suite:exact: [ Failure ]",
	)
	for testCase, matches := range map[TestCase]bool{
		"suite:end:":                    true,
		"suite:end:a=1":                 true,
		"suite:ending:":                 false,
		"suite:middle:a=1;b=2;c=3":      true,
		"suite:middle:a=1;b=3;c=3":      false,
		"suite:middle:a=1;b=2":          false,
		`suite:escaped:op="*"`:          true,
		`suite:escaped:op="+"`:          false,
		`suite:escaped:op="\*"`:         false,
		`suite:escaped_glob:op="*";a=1`: true,
		`suite:escaped_glob:op="+";a=1`: false,
		"suite:exact:":                  true,
		"suite:exact:a=1":               false,
	} {
		require.Equal(t, matches, expectFile.lookup(testCase).results == expectFailure, testCase)
	}
}

// The line that names a test case decides, then the longest wildcard query
// that matches, then the query of that length that comes first in the file,
// counting the lines that do not apply. Lines with the same query are combined.
func TestExpectationPrecedence(t *testing.T) {
	expectFile := parseLines(t, []string{"linux"},
		tagHeader,
		"suite:a:* [ Skip ]",
		"suite:a:b=1;* [ Failure ]",
		"suite:a:b=1;c=1 [ RetryOnFailure ]",
		"suite:a:b=2;* [ Slow ]",
		"suite:a:*;c=3 [ Failure ]",
		"suite:a:b=3;* [ RetryOnFailure ]",
		"suite:a:b=4;* [ Failure ]",
		"[ linux ] suite:a:b=4;* [ RetryOnFailure ]",
		"[ win ] suite:a:b=4;* [ Skip ]",
		"[ win ] suite:c:*;e=1 [ Failure ]",
		"suite:c:d=1;* [ Skip ]",
		"suite:c:*;e=1 [ RetryOnFailure ]",
		"[ win ] suite:d:*;f=1 [ Failure ]",
		"[ win ] suite:d:e=1;* [ Failure ]",
		"suite:d:e=1;* [ Skip ]",
		"suite:d:*;f=1 [ RetryOnFailure ]",
	)
	for testCase, results := range map[TestCase]expectedResults{
		"suite:a:b=0;c=1": expectSkip,
		"suite:a:b=1;c=2": expectFailure,
		"suite:a:b=1;c=1": expectRetryOnFailure,
		"suite:a:b=2;c=1": expectSlow,
		"suite:a:b=3;c=3": expectFailure,
		"suite:a:b=3;c=1": expectRetryOnFailure,
		"suite:a:b=4;c=1": expectFailure | expectRetryOnFailure,
		"suite:c:d=1;e=1": expectRetryOnFailure,
		"suite:d:e=1;f=1": expectRetryOnFailure,
		"suite:b:":        0,
	} {
		require.Equal(t, results, expectFile.lookup(testCase).results, testCase)
	}
}

// Failure expects fail, crash or timeout, RetryOnFailure allows any status,
// Slow allows a timeout, and without any of them a test case is expected not
// to fail. A skip is always allowed.
func TestExpectationUnexpected(t *testing.T) {
	for _, test := range []struct {
		results    expectedResults
		unexpected []Status
	}{
		{0, []Status{Fail, Crash, Timeout}},
		{expectFailure, []Status{Pass, Warn}},
		{expectRetryOnFailure, nil},
		{expectSlow, []Status{Fail, Crash}},
		{expectFailure | expectSlow, []Status{Pass, Warn}},
		{expectFailure | expectRetryOnFailure, nil},
		{expectSkip | expectFailure, nil},
	} {
		unexpected := []Status(nil)
		for _, status := range AllStatuses {
			if test.results.unexpected(status) {
				unexpected = append(unexpected, status)
			}
		}
		require.ElementsMatch(t, test.unexpected, unexpected, "results %b", test.results)
	}
}

// Test cases with a Skip expectation are taken out of the test cases to run
// and get a skip result, numbered after the results of the others.
func TestRemoveSkipped(t *testing.T) {
	state := State{expectFile: parseLines(t, nil,
		"suite:hang:* [ Skip ]",
		"suite:fail:* [ Failure ]",
		"suite:both:* [ Failure ]",
		"suite:both:* [ Skip ]",
	)}
	remaining, skipped := state.RemoveSkipped([]TestCase{"suite:a:", "suite:hang:x=1", "suite:fail:", "suite:both:", "suite:hang:x=2", "suite:b:"})
	require.Equal(t, []TestCase{"suite:a:", "suite:fail:", "suite:b:"}, remaining)
	require.Equal(t, []Result{
		{Index: 3, TestCase: "suite:hang:x=1", Status: Skip, Message: "skipped by expectations.txt:1\n"},
		{Index: 4, TestCase: "suite:both:", Status: Skip, Message: "skipped by expectations.txt:4\n"},
		{Index: 5, TestCase: "suite:hang:x=2", Status: Skip, Message: "skipped by expectations.txt:1\n"},
	}, skipped)
}

// stdoutBuffer is a State.Stdout that keeps what is written to it.
type stdoutBuffer struct {
	bytes.Buffer
}

func (*stdoutBuffer) Close() error { return nil }

// Only the failures that the expectations file does not expect fail the run,
// and every unexpected result is printed.
func TestStreamResultsWithExpectationsFile(t *testing.T) {
	expectFile := parseLines(t, nil,
		"suite:failure:* [ Failure ]",
		"suite:flaky:* [ RetryOnFailure ]",
	)
	streamResults := func(results ...Result) (string, error) {
		stdout := &stdoutBuffer{}
		stream := make(chan Result, len(results))
		for i := range results {
			stream <- results[i]
		}
		close(stream)
		_, err := StreamResults(context.Background(), false, false, State{Stdout: stdout, expectFile: expectFile}, false, nil, len(results), stream, nil)
		return stdout.String(), err
	}
	expected := []Result{
		{TestCase: "suite:a:", Status: Pass},
		{TestCase: "suite:failure:fail", Status: Fail},
		{TestCase: "suite:failure:crash", Status: Crash},
		{TestCase: "suite:failure:timeout", Status: Timeout},
		{TestCase: "suite:flaky:fail", Status: Fail},
		{TestCase: "suite:flaky:pass", Status: Pass},
	}

	stdout, err := streamResults(expected...)
	require.NoError(t, err)
	require.NotContains(t, stdout, "suite:")
	require.NotContains(t, stdout, "unexpected")
	require.NotContains(t, stdout, " FAIL: ")
	require.NotContains(t, stdout, "CRASH: ")
	require.NotContains(t, stdout, "TIMEOUT: ")
	require.Contains(t, stdout, "XFAIL: ")

	stdout, err = streamResults(append(expected, Result{TestCase: "suite:failure:pass", Status: Pass})...)
	require.NoError(t, err)
	require.Contains(t, stdout, "suite:failure:pass - pass [fail -> pass]\n")
	require.Contains(t, stdout, "[1 unexpected]")

	stdout, err = streamResults(append(expected,
		Result{TestCase: "suite:b:", Status: Fail},
		Result{TestCase: "suite:c:", Status: Crash},
		Result{TestCase: "suite:d:", Status: Timeout})...)
	require.EqualError(t, err, "2 unexpected failure(s)")
	require.Contains(t, stdout, "suite:b: - fail\n")
	require.Contains(t, stdout, "suite:c: - crash\n")
	require.Contains(t, stdout, "suite:d: - timeout\n")
}
