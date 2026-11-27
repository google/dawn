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
	"errors"
	"testing"

	"github.com/stretchr/testify/require"
)

// The output of a result is printed from VerboseUnexpected if the result is
// unexpected, from VerboseAll whatever it is, and at every level if it has an
// Error.
func TestVerbosityPrintsOutput(t *testing.T) {
	failed := Result{TestCase: "suite:a:", Status: Fail}
	broken := Result{TestCase: "suite:a:", Status: Fail, Error: errors.New("the runner broke")}
	for _, test := range []struct {
		verbosity  Verbosity
		result     Result
		unexpected bool
		prints     bool
	}{
		{VerboseSummary, failed, true, false},
		{VerboseSummary, broken, false, true},
		{VerboseUnexpected, failed, true, true},
		{VerboseUnexpected, failed, false, false},
		{VerboseUnexpected, broken, false, true},
		{VerboseAll, failed, false, true},
		{VerboseDebug, failed, false, true},
	} {
		require.Equal(t, test.prints, test.verbosity.printsOutput(test.result, test.unexpected), "%+v", test)
	}
}
