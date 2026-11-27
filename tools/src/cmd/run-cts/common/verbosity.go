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
	"strings"
)

// Verbosity is how much run-cts prints. Each level also prints what the levels
// before it print.
type Verbosity int

const (
	VerboseSummary    Verbosity = iota // the lists of unexpected results and the summary
	VerboseUnexpected                  // the output of the test cases with unexpected results
	VerboseAll                         // the output of every test case
	VerboseDebug                       // the commands that run-cts runs, and Dawn's verbose output
)

// verbosityNames holds the name of each level, as -verbose takes it.
var verbosityNames = [...]string{
	VerboseSummary:    "summary",
	VerboseUnexpected: "unexpected",
	VerboseAll:        "all",
	VerboseDebug:      "debug",
}

// String returns the name of the level.
func (v Verbosity) String() string {
	return verbosityNames[v]
}

// Set sets the level to the one that value names. A -verbose flag without a
// value passes "true", which selects VerboseDebug.
func (v *Verbosity) Set(value string) error {
	if value == "true" {
		*v = VerboseDebug
		return nil
	}
	for level, name := range verbosityNames {
		if value == name {
			*v = Verbosity(level)
			return nil
		}
	}
	return fmt.Errorf("must be one of %v", strings.Join(verbosityNames[:], ", "))
}

// IsBoolFlag returns true, which lets the flag be given without a value.
func (v Verbosity) IsBoolFlag() bool {
	return true
}

// Debug returns true at the level that prints the commands that run-cts runs
// and Dawn's verbose output.
func (v Verbosity) Debug() bool {
	return v >= VerboseDebug
}

// printsOutput returns true if the output of result is printed as it finishes:
// at VerboseAll always, at VerboseUnexpected if it is unexpected, and at every
// level if it has an Error.
func (v Verbosity) printsOutput(result Result, unexpected bool) bool {
	return v >= VerboseAll || result.Error != nil || (v >= VerboseUnexpected && unexpected)
}
