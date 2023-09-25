// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package progressbar provides functions for drawing unicode progress bars to
// the terminal
package progressbar

import (
	"bytes"
	"fmt"
	"io"
	"math"
	"strings"
	"time"
)

// Defaults for the Config
const (
	DefaultRefreshRate = time.Millisecond * 100
	DefaultWidth       = 50
	DefaultANSIColors  = true
)

// Config holds configuration options for a ProgressBar
type Config struct {
	RefreshRate time.Duration
	Width       int
	ANSIColors  bool
}

// Color is an enumerator of colors
type Color int

// Color enumerators
const (
	White Color = iota
	Red
	Green
	Yellow
	Blue
	Magenta
	Cyan
)

// Segment describes a single segment of the ProgressBar
type Segment struct {
	Count       int
	Color       Color
	Transparent bool
	Bold        bool
}

// Status holds the updated data of the ProgressBar
type Status struct {
	Total    int
	Segments []Segment
}

// ProgressBar returns a string with an ANSI-colored progress bar, providing
// realtime information about the status of the CTS run.
// Note: We'll want to skip this if !isatty or if we're running on windows.
type ProgressBar struct {
	Config
	out io.Writer
	c   chan Status
}

// New returns a new ProgressBar that streams output to out.
// Call ProgressBar.Stop() once finished.
func New(out io.Writer, cfg *Config) *ProgressBar {
	p := &ProgressBar{out: out, c: make(chan Status, 64)}
	if cfg != nil {
		p.Config = *cfg
	} else {
		p.ANSIColors = DefaultANSIColors
	}
	if p.RefreshRate == 0 {
		p.RefreshRate = DefaultRefreshRate
	}
	if p.Width == 0 {
		p.Width = DefaultWidth
	}
	go func() {
		var status *Status
		t := time.NewTicker(p.RefreshRate)
		defer t.Stop()
		for frame := 0; ; frame++ {
			select {
			case s, ok := <-p.c:
				if !ok {
					return
				}
				status = &s
			case <-t.C:
				if status != nil {
					status.Draw(out, p.Width, p.ANSIColors, frame)
				}
			}
		}
	}()
	return p
}

// Update updates the ProgressBar with the given status
func (p *ProgressBar) Update(s Status) {
	p.c <- s
}

// Stop stops drawing the progress bar.
// Once called, the ProgressBar must not be used.
func (p *ProgressBar) Stop() {
	close(p.c)
}

// Draw draws the ProgressBar status to out
func (s Status) Draw(out io.Writer, width int, ansiColors bool, animFrame int) {
	// ANSI escape sequences
	const (
		escape       = "\u001B["
		positionLeft = escape + "0G"
		ansiReset    = escape + "0m"

		bold = escape + "1m"

		red     = escape + "31m"
		green   = escape + "32m"
		yellow  = escape + "33m"
		blue    = escape + "34m"
		magenta = escape + "35m"
		cyan    = escape + "36m"
		white   = escape + "37m"
	)

	animSymbols := []rune{'⣾', '⣽', '⣻', '⢿', '⡿', '⣟', '⣯', '⣷'}
	blockSymbols := []rune{'▏', '▎', '▍', '▌', '▋', '▊', '▉'}

	numBlocksPrinted := 0

	buf := &bytes.Buffer{}
	fmt.Fprint(buf, "  ", string(animSymbols[animFrame%len(animSymbols)]), " [")

	numFinished := 0
	for _, seg := range s.Segments {
		if ansiColors {
			switch seg.Color {
			case Red:
				buf.WriteString(red)
			case Green:
				buf.WriteString(green)
			case Yellow:
				buf.WriteString(yellow)
			case Blue:
				buf.WriteString(blue)
			case Magenta:
				buf.WriteString(magenta)
			case Cyan:
				buf.WriteString(cyan)
			default:
				buf.WriteString(white)
			}
			if seg.Bold {
				buf.WriteString(bold)
			}
		}

		numFinished += seg.Count
		statusFrac := float64(seg.Count) / float64(s.Total)
		fNumBlocks := float64(width) * statusFrac
		numBlocks := int(math.Ceil(fNumBlocks))
		if seg.Transparent {
			if numBlocks > 0 {
				fmt.Fprint(buf, strings.Repeat(string("░"), numBlocks))
			}
		} else {
			if numBlocks > 1 {
				fmt.Fprint(buf, strings.Repeat(string("▉"), numBlocks-1))
			}
			if numBlocks > 0 {
				frac := fNumBlocks - float64(numBlocks-1)
				symbol := blockSymbols[int(math.Round(frac*float64(len(blockSymbols)-1)))]
				fmt.Fprint(buf, string(symbol))
			}
		}
		numBlocksPrinted += numBlocks
	}

	if width > numBlocksPrinted {
		fmt.Fprint(buf, strings.Repeat(string(" "), width-numBlocksPrinted))
	}
	fmt.Fprint(buf, ansiReset)
	fmt.Fprint(buf, "] ", percentage(numFinished, s.Total))

	if ansiColors {
		// move cursor to start of line so the bar is overridden next print
		fmt.Fprint(buf, positionLeft)
	} else {
		// cannot move cursor, so newline
		fmt.Fprintln(buf)
	}

	out.Write(buf.Bytes())
}

// percentage returns the percentage of n out of total as a string
func percentage(n, total int) string {
	if total == 0 {
		return "-"
	}
	f := float64(n) / float64(total)
	return fmt.Sprintf("%.1f%c", f*100.0, '%')
}
