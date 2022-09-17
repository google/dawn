// Copyright 2022 The Dawn Authors
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

package expectations

import (
	"fmt"
	"io"
	"strings"
)

// Severity is an enumerator of diagnostic severity
type Severity string

const (
	Error   Severity = "error"
	Warning Severity = "warning"
	Note    Severity = "note"
)

// Diagnostic holds a line, column, message and severity.
// Diagnostic also implements the 'error' interface.
type Diagnostic struct {
	Severity Severity
	Line     int // 1-based
	Column   int // 1-based
	Message  string
}

func (e Diagnostic) String() string {
	sb := &strings.Builder{}
	if e.Line > 0 {
		fmt.Fprintf(sb, "%v", e.Line)
		if e.Column > 0 {
			fmt.Fprintf(sb, ":%v", e.Column)
		}
		sb.WriteString(" ")
	}
	sb.WriteString(string(e.Severity))
	sb.WriteString(": ")
	sb.WriteString(e.Message)
	return sb.String()
}

// Error implements the 'error' interface.
func (e Diagnostic) Error() string { return e.String() }

// Diagnostics is a list of diagnostic
type Diagnostics []Diagnostic

// NumErrors returns number of errors in the diagnostics
func (l Diagnostics) NumErrors() int {
	count := 0
	for _, d := range l {
		if d.Severity == Error {
			count++
		}
	}
	return count
}

// Print prints the list of diagnostics to 'w'
func (l Diagnostics) Print(w io.Writer, path string) {
	for _, d := range l {
		fmt.Fprintf(w, "%v:%v %v\n", path, d.Line, d.Message)
	}
}
