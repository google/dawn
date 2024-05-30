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

// fuzz is a helper for running the tint fuzzer executables
package main

import (
	"bytes"
	"context"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"sync/atomic"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"dawn.googlesource.com/dawn/tools/src/term"
	"dawn.googlesource.com/dawn/tools/src/transform"
	"dawn.googlesource.com/dawn/tools/src/utils"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

type fuzzerInfo struct {
	name string // Short name of the fuzzer
	path string // Path to the fuzzer executable
	ext  string // File extensions used by the fuzzer
	dict string // Optional path to a dictionary file for the fuzzer
}

func run() error {
	t := tool{}

	allFuzzers := []fuzzerInfo{
		{
			name: "wgsl",
			path: "tint_wgsl_fuzzer",
			ext:  ".wgsl",
			dict: "src/tint/cmd/fuzz/wgsl/dictionary.txt",
		},
	}
	fuzzerByName := map[string]fuzzerInfo{}
	for _, fuzzer := range allFuzzers {
		fuzzerByName[fuzzer.name] = fuzzer
	}
	allFuzzerNames := transform.SliceNoErr(allFuzzers, func(f fuzzerInfo) string { return f.name })

	check := false
	build := ""
	flag.BoolVar(&t.verbose, "verbose", false, "print additional output")
	flag.BoolVar(&check, "check", false, "check that all the end-to-end test do not fail")
	flag.BoolVar(&t.securityOnly, "security-only", false, "ignore issues that are not considered security impacting")
	flag.StringVar(&t.filter, "filter", "", "filter the fuzzers run to those with this substring")
	flag.StringVar(&t.corpus, "corpus", defaultCorpusDir(), "the corpus directory")
	flag.StringVar(&build, "build", defaultBuildDir(), "the build directory")
	flag.StringVar(&t.out, "out", "<tmp>", "the directory to hold generated test files")
	flag.IntVar(&t.numProcesses, "j", runtime.NumCPU(), "number of concurrent fuzzers to run")
	flag.Usage = func() {
		fmt.Printf(`
fuzz is a helper for running the tint fuzzer executables

fuzz can check that the corpus does not trigger any issues with the fuzzers, and
simplify running of the fuzzers locally.

usage:
	fuzz [fuzzers] [flags...]

fuzzers are the fuzzer types to run, defaults to all.
	Possible values: ` + strings.Join(allFuzzerNames, ", ") + `
`)
		flag.PrintDefaults()
		fmt.Println(``)
		os.Exit(1)
	}
	flag.Parse()

	selectedFuzzers := flag.Args()
	if len(selectedFuzzers) == 0 {
		selectedFuzzers = allFuzzerNames
	}

	if t.numProcesses < 1 {
		t.numProcesses = 1
	}

	if !fileutils.IsDir(build) {
		return fmt.Errorf("build directory '%v' does not exist", build)
	}

	// Verify / create the output directory
	if t.out == "" || t.out == "<tmp>" {
		if tmp, err := os.MkdirTemp("", "tint_fuzz"); err == nil {
			defer os.RemoveAll(tmp)
			t.out = tmp
		} else {
			return err
		}
	}
	if !fileutils.IsDir(t.out) {
		return fmt.Errorf("output directory '%v' does not exist", t.out)
	}

	// Register all the fuzzers
	for _, name := range selectedFuzzers {
		fuzzer, ok := fuzzerByName[name]
		if !ok {
			return fmt.Errorf("unknown fuzzer '%v'. Possible values: %v", name, strings.Join(allFuzzerNames, ", "))
		}

		fuzzer.path = filepath.Join(build, fuzzer.path)
		if !fileutils.IsExe(fuzzer.path) {
			return fmt.Errorf("fuzzer not found at '%v'", fuzzer.path)
		}

		if fuzzer.dict != "" {
			dictPath, err := filepath.Abs(filepath.Join(fileutils.DawnRoot(), fuzzer.dict))
			if err != nil || !fileutils.IsFile(dictPath) {
				return fmt.Errorf("failed to obtain the dictionary.txt path: %w", err)
			}
			fuzzer.dict = dictPath
		}

		t.fuzzers = append(t.fuzzers, fuzzer)
	}

	if dxc := filepath.Join(build, dxcFileName()); fileutils.IsFile(dxc) {
		t.dxc = dxc
	}

	// If --check was passed, then just ensure that all the files in the corpus
	// directory don't upset the fuzzers
	if check {
		return t.check()
	}

	// Run the fuzzers
	return t.run()
}

type tool struct {
	verbose      bool
	filter       string       // filter fuzzers to those with this substring
	corpus       string       // directory of test files
	out          string       // where to emit new test files
	dxc          string       // path to the DXC DLL / so
	fuzzers      []fuzzerInfo // the fuzzers to run
	numProcesses int          // number of concurrent processes to spawn
	securityOnly bool         // Ignore non-security crashes
}

// check() runs the fuzzers against all the .wgsl files under to the corpus directory,
// ensuring that the fuzzers do not error for the given file.
func (t tool) check() error {
	type job struct {
		file string
		exe  string
	}

	jobs := []job{}

	for _, fuzzer := range t.fuzzers {
		files, err := t.fuzzerCorpusFiles(fuzzer)
		if err != nil {
			return err
		}

		log.Printf("%v: checking %v files...\n", fuzzer.name, len(files))

		for _, file := range files {
			jobs = append(jobs, job{file: file, exe: fuzzer.path})
		}
	}

	remaining := transform.SliceToChan(jobs)

	var pb *progressbar.ProgressBar
	if term.CanUseAnsiEscapeSequences() {
		pb = progressbar.New(os.Stdout, nil)
		defer pb.Stop()
	}
	var numDone uint32

	routine := func() error {
		for job := range remaining {
			atomic.AddUint32(&numDone, 1)
			if pb != nil {
				pb.Update(progressbar.Status{
					Total: len(jobs),
					Segments: []progressbar.Segment{
						{Count: int(atomic.LoadUint32(&numDone))},
					},
				})
			}

			args := []string{}
			if t.dxc != "" {
				args = append(args, "--dxc="+t.dxc)
			}
			args = append(args, job.file)

			if out, err := exec.Command(job.exe, args...).CombinedOutput(); err != nil {
				_, fuzzer := filepath.Split(job.exe)
				return fmt.Errorf("%v run with %v failed with %v\n\n%v", fuzzer, job, err, string(out))
			}
		}
		return nil
	}

	if err := utils.RunConcurrent(t.numProcesses, routine); err != nil {
		return err
	}

	log.Printf("done")
	return nil
}

// run() runs the fuzzers across t.numProcesses processes.
// The fuzzers will use t.corpus as the seed directory.
// New cases are written to t.out.
// Blocks until a fuzzer errors, or the process is interrupted.
func (t tool) run() error {
	// Regular expression used to identify the crash file written by libfuzzer
	var reCrashFile = regexp.MustCompile("crash-[a-z0-9]{40}")

	ctx := utils.CancelOnInterruptContext(context.Background())
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	routinesPerFuzzer := t.numProcesses / len(t.fuzzers)
	if routinesPerFuzzer == 0 {
		routinesPerFuzzer = 1
	}

	errs := make(chan error, 8)

	for _, fuzzer := range t.fuzzers {
		fuzzer := fuzzer

		corpusFiles, err := t.fuzzerCorpusFiles(fuzzer)
		if err != nil {
			return err
		}

		log.Println("copying", len(corpusFiles), fuzzer.ext, "files to", t.out+"...")
		for _, path := range corpusFiles {
			_, file := filepath.Split(path)
			if err := fileutils.CopyFile(filepath.Join(t.out, file), path); err != nil {
				return err
			}
		}

		args := []string{t.out}
		if fuzzer.dict != "" {
			args = append(args, "-dict="+fuzzer.dict)
		}
		if t.verbose {
			args = append(args, "--verbose")
		}
		if t.filter != "" {
			args = append(args, "--filter="+t.filter)
		}
		if t.dxc != "" {
			args = append(args, "--dxc="+t.dxc)
		}

		log.Println("running", routinesPerFuzzer, fuzzer.name, "fuzzer instances...")
		for i := 0; i < routinesPerFuzzer; i++ {
			go func() {
				for {
					cmd := exec.CommandContext(ctx, fuzzer.path, args...)
					out := bytes.Buffer{}
					cmd.Stdout = &out
					cmd.Stderr = &out
					if t.verbose {
						cmd.Stdout = io.MultiWriter(&out, os.Stdout)
						cmd.Stderr = io.MultiWriter(&out, os.Stderr)
					}
					if err := cmd.Run(); err != nil {
						if ctxErr := ctx.Err(); ctxErr != nil {
							errs <- ctxErr
						} else {
							if t.securityOnly && isFailureNonSecurity(out.String()) {
								log.Println("non-security crash found. restarting...")
								if file := reCrashFile.FindString(out.String()); file != "" {
									os.Remove(file)
								}
								continue
							}
							_, fuzzer := filepath.Split(fuzzer.ext)
							errs <- fmt.Errorf("%v failed with %v\n\n%v", fuzzer, err, out.String())
						}
					} else {
						errs <- fmt.Errorf("fuzzer unexpectedly terminated without error:\n%v", out.String())
					}
					break
				}
			}()
		}
	}

	for err := range errs {
		return err
	}
	return nil
}

func (t tool) fuzzerCorpusFiles(f fuzzerInfo) ([]string, error) {
	files, err := glob.Glob(filepath.Join(t.corpus, "**"+f.ext))
	if err != nil {
		return nil, err
	}

	// Remove '*.expected.wgsl'
	if f.name == "wgsl" {
		files = transform.Filter(files, func(s string) bool { return !strings.Contains(s, ".expected.wgsl") })
	}

	return files, nil
}

func defaultCorpusDir() string {
	return filepath.Join(fileutils.DawnRoot(), "test/tint")
}

func defaultBuildDir() string {
	return filepath.Join(fileutils.DawnRoot(), "out/active")
}

func dxcFileName() string {
	switch runtime.GOOS {
	case "windows":
		return "dxcompiler.dll"
	case "darwin":
		return "libdxcompiler.dylib"
	default:
		return "libdxcompiler.so"
	}
}

func isFailureNonSecurity(out string) bool {
	for _, str := range []string{
		"AddressSanitizer: SEGV on unknown address 0x000000000000",
		"ICE while running fuzzer",
	} {
		if strings.Contains(out, str) {
			return true
		}
	}
	return false
}
