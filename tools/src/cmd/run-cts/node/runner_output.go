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
	"bufio"
	"io"
	"regexp"
	"strconv"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
)

// resultLine matches the line that cmdline.js --verbose prints when a test
// case finishes, e.g. `[pass] webgpu:a:b:c=1 (12.5ms). Log:`. The case name is
// everything between the status and the last " (<time>ms). Log:", so it may
// contain any character.
var resultLine = regexp.MustCompile(`^\[([a-z]+)\] (.+) \(([^()\s]+)ms\)\. Log:$`)

// parseResultLine returns the status word, test case and duration of a result
// line, or ok=false if line is not a result line.
func parseResultLine(line string) (status string, testCase common.TestCase, duration time.Duration, ok bool) {
	match := resultLine.FindStringSubmatch(line)
	if match == nil {
		return "", "", 0, false
	}
	ms, err := strconv.ParseFloat(match[3], 64)
	if err != nil {
		return "", "", 0, false
	}
	return match[1], common.TestCase(match[2]), time.Duration(ms * float64(time.Millisecond)), true
}

// toStatus converts a status word printed by cmdline.js into a Status.
func toStatus(word string) (common.Status, bool) {
	switch word {
	case "pass":
		return common.Pass, true
	case "fail":
		return common.Fail, true
	case "warn":
		return common.Warn, true
	case "skip":
		return common.Skip, true
	}
	return common.Fail, false
}

// parseCoverageLine returns the coverage file path printed by cmdline.js
// --coverage, or ok=false if line is not a coverage line.
func parseCoverageLine(line string) (string, bool) {
	rest, ok := strings.CutPrefix(line, "Code-coverage: [[")
	if !ok {
		return "", false
	}
	return strings.CutSuffix(rest, "]]")
}

// isLogLine returns true if line is part of a test case's log, which
// cmdline.js prints after the result line with every line indented.
func isLogLine(line string) bool {
	return strings.HasPrefix(line, "  ")
}

// joinLines returns lines as text with a newline after each line.
func joinLines(lines []string) string {
	if len(lines) == 0 {
		return ""
	}
	return strings.Join(lines, "\n") + "\n"
}

// readLines sends each line read from r to lines, without the line ending,
// then closes lines.
func readLines(r io.Reader, lines chan<- string) {
	defer close(lines)
	reader := bufio.NewReader(r)
	for {
		line, err := reader.ReadString('\n')
		if line != "" {
			lines <- strings.TrimRight(line, "\r\n")
		}
		if err != nil {
			return
		}
	}
}
