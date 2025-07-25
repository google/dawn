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
	"runtime"
	"strings"
	"sync/atomic"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"dawn.googlesource.com/dawn/tools/src/term"
	"dawn.googlesource.com/dawn/tools/src/transform"
	"dawn.googlesource.com/dawn/tools/src/utils"
)

const (
	wgslDictionaryRelPath = "src/tint/cmd/fuzz/wgsl/dictionary.txt"
)

func main() {
	if err := run(oswrapper.GetRealOSWrapper()); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
fuzz is a helper for running the tint fuzzer executables

fuzz can check that the corpus does not trigger any issues with the fuzzers, and
simplify running of the fuzzers locally.

usage:
  fuzz [flags...]`)
	flag.PrintDefaults()
	fmt.Println(``)
	os.Exit(1)
}

// TODO(crbug.com/344014313): Add unittests once fileutils and term are updated
// to support dependency injection.
func run(osWrapper oswrapper.OSWrapper) error {
	t := tool{}

	check := false
	build := ""
	flag.BoolVar(&t.verbose, "verbose", false, "print additional output")
	flag.BoolVar(&check, "check", false, "check that all the end-to-end test do not fail")
	flag.BoolVar(&t.dump, "dump", false, "dumps shader input/output from config")
	flag.BoolVar(&t.irMode, "ir", false, "runs using IR config instead of WGSL config (This feature is a WIP)")
	flag.StringVar(&t.filter, "filter", "", "filter the fuzzers run to those with this substring")
	flag.StringVar(&t.corpus, "corpus", defaultWgslCorpusDir(osWrapper), "the corpus directory")
	flag.StringVar(&build, "build", defaultBuildDir(osWrapper), "the build directory")
	flag.StringVar(&t.out, "out", "<tmp>", "the directory to hold generated test files")
	flag.IntVar(&t.numProcesses, "j", runtime.NumCPU(), "number of concurrent fuzzers to run")
	flag.Parse()

	if t.numProcesses < 1 {
		t.numProcesses = 1
	}

	if !fileutils.IsDir(build) {
		return fmt.Errorf("build directory '%v' does not exist", build)
	}

	if t.irMode && t.corpus == defaultWgslCorpusDir(osWrapper) {
		t.corpus = ""
	}

	// Verify / create the output directory
	if t.out == "" || t.out == "<tmp>" {
		if tmp, err := osWrapper.MkdirTemp("", "tint_fuzz"); err == nil {
			defer osWrapper.RemoveAll(tmp)
			t.out = tmp
		} else {
			return err
		}
	}
	if !fileutils.IsDir(t.out) {
		return fmt.Errorf("output directory '%v' does not exist", t.out)
	}

	fuzzers := make([]fuzzerConfig, 0, 1)
	if t.irMode {
		fuzzers = append(fuzzers, fuzzerConfig{"tint_ir_fuzzer", &t.fuzzer})
	} else {
		fuzzers = append(fuzzers, fuzzerConfig{"tint_wgsl_fuzzer", &t.fuzzer})
	}

	// Verify all the config executables are present
	for _, config := range fuzzers {
		*config.path = filepath.Join(build, config.name+fileutils.ExeExt)
		if !fileutils.IsExe(*config.path) {
			return fmt.Errorf("config '%v' not found at '%v'", config.name, *config.path)
		}
	}

	// If --check was passed, then just ensure that all the files in the corpus
	// directory don't upset the fuzzers
	if check {
		return t.check(osWrapper)
	}

	// Run the fuzzers
	return t.run(osWrapper)
}

type fuzzerConfig struct {
	name string  // name of the fuzzer binary
	path *string // path to fuzzer binary
}

type tool struct {
	verbose      bool   // prints the name of each fuzzer before running
	dump         bool   // dumps shader input/output from fuzzer
	irMode       bool   // run using the IR fuzzer instead of the WGSL fuzzer
	filter       string // filter fuzzers to those with this substring
	corpus       string // directory of test files
	out          string // where to emit new test files
	fuzzer       string // path to fuzzer to run
	numProcesses int    // number of concurrent processes to spawn
}

// TODO(crbug.com/344014313): Add unittests once term is converted to support
// dependency injection.
// check() runs the fuzzer against all the test files the corpus directory,
// ensuring that the fuzzers do not error for the given file.
func (t tool) check(osWrapper oswrapper.OSWrapper) error {
	var files []string
	if t.irMode {
		if t.corpus == "" {
			return fmt.Errorf("must explicitly provide a corpus when running in IR mode")
		}

		var err error
		files, err = glob.Glob(filepath.Join(t.corpus, "**.tirb"), osWrapper)
		if err != nil {
			return err
		}
	} else {
		var err error
		files, err = glob.Glob(filepath.Join(t.corpus, "**.wgsl"), osWrapper)
		if err != nil {
			return err
		}

		// Remove '*.expected.wgsl'
		files = transform.Filter(files, func(s string) bool { return !strings.Contains(s, ".expected.") })
	}
	log.Printf("checking %v files...\n", len(files))

	remaining := transform.SliceToChan(files)

	var pb *progressbar.ProgressBar
	if term.CanUseAnsiEscapeSequences() {
		pb = progressbar.New(os.Stdout, nil)
		defer pb.Stop()
	}
	var numDone uint32

	routine := func() error {
		for file := range remaining {
			atomic.AddUint32(&numDone, 1)
			if pb != nil {
				pb.Update(progressbar.Status{
					Total: len(files),
					Segments: []progressbar.Segment{
						{Count: int(atomic.LoadUint32(&numDone))},
					},
				})
			}

			if out, err := exec.Command(t.fuzzer, file).CombinedOutput(); err != nil {
				_, fuzzer := filepath.Split(t.fuzzer)
				return fmt.Errorf("%v run with %v failed with %v\n\n%v", fuzzer, file, err, string(out))
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

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.
// run() runs the fuzzer across t.numProcesses processes.
// The fuzzer will use t.corpus as the seed directory.
// New cases are written to t.out.
// Blocks until a fuzzer errors, or the process is interrupted.
func (t tool) run(fsReader oswrapper.FilesystemReader) error {
	ctx := utils.CancelOnInterruptContext(context.Background())
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	args, err := t.generateFuzzerArgs(fsReader)
	if err != nil {
		return err
	}

	fmt.Println("running", t.numProcesses, "fuzzer instances")
	errs := make(chan error, t.numProcesses)
	for i := 0; i < t.numProcesses; i++ {
		go func() {
			cmd := exec.CommandContext(ctx, t.fuzzer, args...)
			out := bytes.Buffer{}
			cmd.Stdout = &out
			cmd.Stderr = &out
			if t.verbose || t.dump {
				cmd.Stdout = io.MultiWriter(&out, os.Stdout)
				cmd.Stderr = io.MultiWriter(&out, os.Stderr)
			}
			if err := cmd.Run(); err != nil {
				if ctxErr := ctx.Err(); ctxErr != nil {
					errs <- ctxErr
				} else {
					_, fuzzer := filepath.Split(t.fuzzer)
					errs <- fmt.Errorf("%v failed with %v\n\n%v", fuzzer, err, out.String())
				}
			} else {
				errs <- fmt.Errorf("fuzzer unexpectedly terminated without error:\n%v", out.String())
			}
		}()
	}
	for err := range errs {
		return err
	}
	return nil
}

// TODO(crbug.com/344014313): Add unittests once fileutils is converted to use
// dependency injection.
// generateFuzzerArgs() generates the arguments that need to be passed into the fuzzer binary call
func (t tool) generateFuzzerArgs(fsReader oswrapper.FilesystemReader) ([]string, error) {
	args := []string{t.out}

	if t.corpus != "" {
		args = append(args, t.corpus)
	}
	if !t.irMode {
		dictPath, err := filepath.Abs(filepath.Join(fileutils.DawnRoot(fsReader), wgslDictionaryRelPath))
		if err != nil || !fileutils.IsFile(dictPath) {
			return []string{}, fmt.Errorf("failed to obtain the dictionary.txt path: %w", err)
		}
		args = append(args, "-dict="+dictPath)
	}
	if t.verbose {
		args = append(args, "--verbose")
	}
	if t.dump {
		args = append(args, "--dump")
	}
	if t.filter != "" {
		args = append(args, "--filter="+t.filter)
	}
	return args, nil
}

func defaultWgslCorpusDir(fsReader oswrapper.FilesystemReader) string {
	return filepath.Join(fileutils.DawnRoot(fsReader), "test", "tint")
}

func defaultBuildDir(fsReader oswrapper.FilesystemReader) string {
	return filepath.Join(fileutils.DawnRoot(fsReader), "out", "active")
}
