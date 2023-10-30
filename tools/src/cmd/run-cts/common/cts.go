// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
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
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

type CTS struct {
	path string                   // Path to the CTS directory
	npx  string                   // Path to npx executable (optional)
	node string                   // Path to node executable
	eval func(main string) string // Returns JavaScript to run the given typescript tool
}

func NewCTS(cts, npx, node string) CTS {
	return CTS{path: cts, npx: npx, node: node, eval: evalScriptUsingJIT}
}

// Eval returns the JavaScript to run the given typescript tool
func (c CTS) Eval(main string) string {
	return c.eval(main)
}

// BuildIfRequired calls Build() if the CTS sources have been modified since the
// last build.
func (c CTS) BuildIfRequired(verbose bool) error {
	if c.npx == "" {
		fmt.Println("npx not found on PATH. Using runtime TypeScript transpilation (slow)")
		c.eval = evalScriptUsingJIT
		return nil
	}

	// Scan the CTS source to determine the most recent change to the CTS source
	mostRecentSourceChange, err := scanSourceTimestamps(filepath.Join(c.path, "src"), verbose)
	if err != nil {
		return fmt.Errorf("failed to scan source files for modified timestamps: %w", err)
	}

	type Cache struct {
		BuildTimestamp time.Time
	}

	cachePath := ""
	if home, err := os.UserHomeDir(); err == nil {
		cacheDir := filepath.Join(home, ".cache/webgpu")
		cachePath = filepath.Join(cacheDir, "run-cts.json")
		os.MkdirAll(cacheDir, 0777)
	}

	needsRebuild := true
	if cachePath != "" { // consult the cache to see if we need to rebuild
		if cacheFile, err := os.Open(cachePath); err == nil {
			cache := Cache{}
			if err := json.NewDecoder(cacheFile).Decode(&cache); err == nil {
				if fileutils.IsDir(filepath.Join(c.path, "out-node")) {
					needsRebuild = mostRecentSourceChange.After(cache.BuildTimestamp)
				}
			}
			cacheFile.Close()
		}
	}

	if verbose {
		fmt.Println("CTS needs rebuild:", needsRebuild)
	}

	if needsRebuild {
		if err := c.Build(verbose); err != nil {
			return fmt.Errorf("failed to build CTS: %w", err)
		}
	}

	if cachePath != "" { // consult the cache to see if we need to rebuild
		if cacheFile, err := os.Create(cachePath); err == nil {
			c := Cache{BuildTimestamp: mostRecentSourceChange}
			json.NewEncoder(cacheFile).Encode(&c)
			cacheFile.Close()
		}
	}

	return nil
}

// Build calls `npx grunt run:build-out-node` in the CTS directory to compile
// the TypeScript files down to JavaScript. Doing this once ahead of time can be
// much faster than dynamically transpiling when there are many tests to run.
func (c CTS) Build(verbose bool) error {
	if verbose {
		start := time.Now()
		fmt.Println("Building CTS...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	cmd := exec.Command(c.npx, "grunt", "run:build-out-node")
	cmd.Dir = c.path
	out, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("%w: %v", err, string(out))
	}

	// Can evaluate with the faster .js pre-built
	c.eval = evalUsingCompiled
	return nil
}

// QueryTestCases returns all the test cases that match query.
func (n *CTS) QueryTestCases(verbose bool, query string) ([]TestCase, error) {
	if verbose {
		start := time.Now()
		fmt.Println("Gathering test cases...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	args := append([]string{
		"-e", n.Eval("cmdline"),
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		"--list",
	}, query)

	cmd := exec.Command(n.node, args...)
	cmd.Dir = n.path
	out, err := cmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("%w\n%v", err, string(out))
	}

	lines := strings.Split(string(out), "\n")
	list := make([]TestCase, 0, len(lines))
	for _, line := range lines {
		if line != "" {
			list = append(list, TestCase(line))
		}
	}
	return list, nil
}

func evalUsingCompiled(main string) string {
	return fmt.Sprintf(`require('./out-node/common/runtime/%v.js');`, main)
}

func evalScriptUsingJIT(main string) string {
	return fmt.Sprintf(`require('./src/common/tools/setup-ts-in-node.js');require('./src/common/runtime/%v.ts');`, main)
}

// scanSourceTimestamps scans all the .js and .ts files in all subdirectories of
// dir, and returns the file with the most recent timestamp.
func scanSourceTimestamps(dir string, verbose bool) (time.Time, error) {
	if verbose {
		start := time.Now()
		fmt.Println("Scanning .js / .ts files for changes...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	mostRecentChange := time.Time{}
	err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		switch filepath.Ext(path) {
		case ".ts", ".js":
			if info.ModTime().After(mostRecentChange) {
				mostRecentChange = info.ModTime()
			}
		}
		return nil
	})
	if err != nil {
		return time.Time{}, err
	}
	return mostRecentChange, nil
}
