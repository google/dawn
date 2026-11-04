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

package progressbar

import (
	"bytes"
	"strings"
	"testing"
	"time"

	"github.com/stretchr/testify/require"
)

func TestRealProgressBarFlushesFinalStatusOnStop(t *testing.T) {
	var buf bytes.Buffer
	// Use a very long refresh rate so the ticker will not fire before Stop().
	pb := newRealProgressBar(&buf, &Config{
		RefreshRate: time.Hour,
		Width:       10,
		ANSIColors:  true,
	})

	pb.Update(Status{
		Total:    10,
		Segments: []Segment{{Count: 5}},
	})
	pb.Update(Status{
		Total:    10,
		Segments: []Segment{{Count: 10}},
	})

	pb.Stop()
	// Calling Stop() a second time should be a safe no-op.
	pb.Stop()

	out := buf.String()
	require.Contains(t, out, "100.0%")
	require.True(t, strings.HasSuffix(out, "\n"), "expected trailing newline after Stop(), got %q", out)
}

func TestRealProgressBarStopWithNoUpdates(t *testing.T) {
	var buf bytes.Buffer
	pb := newRealProgressBar(&buf, &Config{
		RefreshRate: time.Hour,
		Width:       10,
		ANSIColors:  true,
	})

	pb.Stop()
	require.Empty(t, buf.String())
}

func TestRealProgressBarNoANSIColors(t *testing.T) {
	var buf bytes.Buffer
	pb := newRealProgressBar(&buf, &Config{
		RefreshRate: time.Hour,
		Width:       10,
		ANSIColors:  false,
	})

	pb.Update(Status{
		Total:    10,
		Segments: []Segment{{Count: 10}},
	})
	pb.Stop()

	out := buf.String()
	require.Equal(t, "  ⣾ [▉▉▉▉▉▉▉▉▉▉\u001b[0m] 100.0%\n", out)
}
