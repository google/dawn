// Copyright 2022 The Tint Authors.
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

// tint-bench repeatedly emits a WGSL file from a template, then times how long
// it takes to execute the tint executable with that WGSL file.
package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/template"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

// [from .. to]
type Range struct {
	from int
	to   int
}

func run() error {
	alphaRange := Range{}

	iterations := 0
	tmplPath := ""
	flag.StringVar(&tmplPath, "tmpl", "tint-bench.tmpl", `the WGSL template to benchmark.
Searches in order: absolute, relative to CWD, then relative to `+fileutils.ThisDir())
	flag.IntVar(&alphaRange.from, "alpha-from", 0, "the start value for 'Alpha'")
	flag.IntVar(&alphaRange.to, "alpha-to", 10, "the end value for 'Alpha'")
	flag.IntVar(&iterations, "iterations", 10, "number of times to benchmark tint")
	flag.Usage = func() {
		fmt.Println("tint-bench repeatedly emits a WGSL file from a template, then times how long")
		fmt.Println("it takes to execute the tint executable with that WGSL file.")
		fmt.Println("")
		fmt.Println("usage:")
		fmt.Println("  tint-bench <bench-flags> [tint-exe] <tint-flags>")
		fmt.Println("")
		fmt.Println("bench-flags:")
		flag.PrintDefaults()
		os.Exit(1)
	}

	flag.Parse()

	if tmplPath == "" {
		return fmt.Errorf("missing template path")
	}

	tmpl, err := ioutil.ReadFile(tmplPath)
	if err != nil {
		if !filepath.IsAbs(tmplPath) {
			// Try relative to this .go file
			tmplPath = filepath.Join(fileutils.ThisDir(), tmplPath)
			tmpl, err = ioutil.ReadFile(tmplPath)
		}
	}
	if err != nil {
		return fmt.Errorf("failed to load template: %v", err)
	}

	args := flag.Args()
	if len(args) < 1 {
		flag.Usage()
	}
	tintExe := args[0]

	fmt.Println(" alpha | Time (μs)")
	fmt.Println("-------+-----------------")

	for alpha := alphaRange.from; alpha < alphaRange.to; alpha++ {
		alpha := alpha
		funcs := template.Functions{
			"Alpha": func() int { return alpha },
		}
		wgslPath, err := writeWGSLFile(string(tmpl), funcs)
		if err != nil {
			return err
		}

		tintArgs := []string{wgslPath}
		tintArgs = append(tintArgs, args[1:]...)

		durations := []time.Duration{}
		for i := 0; i < iterations; i++ {
			tint := exec.Command(tintExe, tintArgs...)
			start := time.Now()
			if out, err := tint.CombinedOutput(); err != nil {
				return fmt.Errorf("tint failed with error: %w\n%v\n\nwith: Alpha=%v", err, string(out), alpha)
			}
			duration := time.Since(start)
			durations = append(durations, duration)
		}
		sort.Slice(durations, func(i, j int) bool { return durations[i] < durations[j] })

		median := durations[len(durations)/2]
		fmt.Printf("%6.v | %v\n", alpha, median.Microseconds())
	}
	return nil
}

func writeWGSLFile(tmpl string, funcs template.Functions) (string, error) {
	const path = "tint-bench.wgsl"
	wgslFile, err := os.Create(path)
	if err != nil {
		return "", fmt.Errorf("failed to create benchmark WGSL test file: %w", err)
	}
	defer wgslFile.Close()
	if err := template.Run(tmpl, wgslFile, funcs); err != nil {
		return "", fmt.Errorf("template error:\n%w", err)
	}
	return path, nil
}
