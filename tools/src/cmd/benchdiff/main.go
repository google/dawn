// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// benchdiff is a tool that compares two Google benchmark results and displays
// sorted performance differences.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"text/tabwriter"
	"time"

	"dawn.googlesource.com/tint/tools/src/bench"
)

var (
	minDiff    = flag.Duration("min-diff", time.Microsecond*10, "Filter away time diffs less than this duration")
	minRelDiff = flag.Float64("min-rel-diff", 0.01, "Filter away absolute relative diffs between [1, 1+x]")
)

func main() {
	flag.ErrHelp = errors.New("benchdiff is a tool to compare two benchmark results")
	flag.Parse()
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, "benchdiff <benchmark-a> <benchmark-b>")
		flag.PrintDefaults()
	}

	args := flag.Args()
	if len(args) < 2 {
		flag.Usage()
		os.Exit(1)
	}

	pathA, pathB := args[0], args[1]

	if err := run(pathA, pathB); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(-1)
	}
}

func run(pathA, pathB string) error {
	fileA, err := ioutil.ReadFile(pathA)
	if err != nil {
		return err
	}
	benchA, err := bench.Parse(string(fileA))
	if err != nil {
		return err
	}

	fileB, err := ioutil.ReadFile(pathB)
	if err != nil {
		return err
	}
	benchB, err := bench.Parse(string(fileB))
	if err != nil {
		return err
	}

	compare(benchA, benchB, fileName(pathA), fileName(pathB))

	return nil
}

func fileName(path string) string {
	_, name := filepath.Split(path)
	return name
}

func compare(benchA, benchB bench.Benchmark, nameA, nameB string) {
	type times struct {
		a time.Duration
		b time.Duration
	}
	byName := map[string]times{}
	for _, test := range benchA.Tests {
		byName[test.Name] = times{a: test.Duration}
	}
	for _, test := range benchB.Tests {
		t := byName[test.Name]
		t.b = test.Duration
		byName[test.Name] = t
	}

	type delta struct {
		name       string
		times      times
		relDiff    float64
		absRelDiff float64
	}
	deltas := []delta{}
	for name, times := range byName {
		if times.a == 0 || times.b == 0 {
			continue // Assuming test was missing from a or b
		}
		diff := times.b - times.a
		absDiff := diff
		if absDiff < 0 {
			absDiff = -absDiff
		}
		if absDiff < *minDiff {
			continue
		}

		relDiff := float64(times.b) / float64(times.a)
		absRelDiff := relDiff
		if absRelDiff < 1 {
			absRelDiff = 1.0 / absRelDiff
		}
		if absRelDiff < (1.0 + *minRelDiff) {
			continue
		}

		d := delta{
			name:       name,
			times:      times,
			relDiff:    relDiff,
			absRelDiff: absRelDiff,
		}
		deltas = append(deltas, d)
	}

	sort.Slice(deltas, func(i, j int) bool { return deltas[j].relDiff < deltas[i].relDiff })

	fmt.Println("A:", nameA)
	fmt.Println("B:", nameB)
	fmt.Println()

	buf := strings.Builder{}
	{
		w := tabwriter.NewWriter(&buf, 1, 1, 0, ' ', 0)
		fmt.Fprintln(w, "Test name\t | Δ (A → B)\t | % (A → B)\t | % (B → A)\t | × (A → B)\t | × (B → A)\t | A \t | B")
		fmt.Fprintln(w, "\t-+\t-+\t-+\t-+\t-+\t-+\t-+\t-")
		for _, delta := range deltas {
			a2b := delta.times.b - delta.times.a
			fmt.Fprintf(w, "%v \t | %v \t | %+2.1f%% \t | %+2.1f%% \t | %+.4f \t | %+.4f \t | %v \t | %v \t|\n",
				delta.name,
				a2b, // Δ (A → B)
				100*float64(a2b)/float64(delta.times.a),       // % (A → B)
				100*float64(-a2b)/float64(delta.times.b),      // % (B → A)
				float64(delta.times.b)/float64(delta.times.a), // × (A → B)
				float64(delta.times.a)/float64(delta.times.b), // × (B → A)
				delta.times.a, // A
				delta.times.b, // B
			)
		}
		w.Flush()
	}

	// Split the table by line so we can add in a header line
	lines := strings.Split(buf.String(), "\n")
	fmt.Println(lines[0])
	fmt.Println(strings.ReplaceAll(lines[1], " ", "-"))
	for _, l := range lines[2:] {
		fmt.Println(l)
	}
}
