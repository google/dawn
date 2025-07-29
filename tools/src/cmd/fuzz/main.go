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

type TaskMode int

const (
	TaskModeRun TaskMode = iota
	TaskModeCheck
	TaskModeGenerate
)

type FuzzMode int

const (
	FuzzModeWgsl FuzzMode = iota
	FuzzModeIr
)

type cmdConfig struct {
	verbose      bool
	dump         bool
	fuzzMode     FuzzMode
	taskMode     TaskMode
	filter       string
	inputs       string
	build        string
	out          string
	numProcesses int
}

func showUsage() {
	out := flag.CommandLine.Output()
	_, _ = fmt.Fprintln(out, `
fuzz is a helper for running the tint fuzzer executables and other related tasks

fuzz has 3, mutually exclusive, tasks that it can perform:
1. Run a fuzzer locally, requires no additional flag.
2. Check that a fuzzer successfully handles contents of -inputs, requires -check flag
3. Generate a fuzzer corpus based on contents of -inputs, requires -generate flag

usage:
  fuzz [flags...]`)
	flag.PrintDefaults()
	_, _ = fmt.Fprintln(out, ``)
}

func main() {
	c := cmdConfig{}
	osWrapper := oswrapper.GetRealOSWrapper()

	flag.Usage = showUsage

	check, generate, irMode := false, false, false
	flag.BoolVar(&c.verbose, "verbose", false, "print additional output")
	flag.BoolVar(&check, "check", false, "check that all the end-to-end tests in -inputs do not fail")
	flag.BoolVar(&generate, "generate", false, "generate fuzzing corpus based on -inputs")
	flag.BoolVar(&c.dump, "dump", false, "dumps shader input/output from fuzzer")
	flag.BoolVar(&irMode, "ir", false, "runs using IR fuzzer instead of WGSL fuzzer (This feature is a WIP)")
	flag.StringVar(&c.filter, "filter", "", "filter the fuzzing passes run to those with this substring")
	flag.StringVar(&c.inputs, "corpus", defaultWgslCorpusDir(osWrapper), "obsolete, use -inputs instead")
	flag.StringVar(&c.inputs, "inputs", defaultWgslCorpusDir(osWrapper), "the directory that holds the files to use")
	flag.StringVar(&c.build, "build", defaultBuildDir(osWrapper), "the build directory")
	flag.StringVar(&c.out, "out", "<tmp>", "the directory to store outputs to")
	flag.IntVar(&c.numProcesses, "j", runtime.NumCPU(), "number of concurrent fuzzers to run")
	flag.Parse()

	if check && generate {
		fmt.Println("cannot set check and generate flags at the same time")
		os.Exit(1)
	}

	switch {
	case check:
		c.taskMode = TaskModeCheck
	case generate:
		c.taskMode = TaskModeGenerate
	default:
		c.taskMode = TaskModeRun
	}

	if irMode {
		c.fuzzMode = FuzzModeIr
	} else {
		c.fuzzMode = FuzzModeWgsl
	}

	if c.numProcesses < 1 {
		c.numProcesses = 1
	}

	// Running/checking the fuzzer in IR mode requires an explicit input directory, and test/tint should not contain any .tirb files.
	if c.inputs == defaultWgslCorpusDir(osWrapper) {
		if c.fuzzMode == FuzzModeIr && (c.taskMode == TaskModeRun || c.taskMode == TaskModeCheck) {
			fmt.Println("-inputs is required when running or checking the IR fuzzer")
			os.Exit(1)
		}
	}

	if err := run(&c, osWrapper); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

type taskConfig struct {
	cmdConfig
	fuzzer     string // path to the fuzzer binary, tint_wgsl_fuzzer or tint_ir_fuzzer
	generator  string // path to the corpus generator, generate_tint_corpus.py
	assembler  string // path to the test case assembler, tint_fuzz_as
	dictionary string // path to dictionary to use for tint_wgsl_fuzzer
}

// TODO(crbug.com/344014313): Add unittests once fileutils and term are updated
// to support dependency injection.
func run(c *cmdConfig, osWrapper oswrapper.OSWrapper) error {
	t := taskConfig{
		cmdConfig: *c,
	}

	if !fileutils.IsDir(t.build) {
		return fmt.Errorf("build directory '%v' does not exist", t.build)
	}

	// Verify / create the output directory
	if t.out == "" || t.out == "<tmp>" {
		if tmp, err := osWrapper.MkdirTemp("", "tint_fuzz"); err == nil {
			defer func(osWrapper oswrapper.OSWrapper, path string) {
				_ = osWrapper.RemoveAll(path)
			}(osWrapper, tmp)
			t.out = tmp
		} else {
			return err
		}
	}

	if !fileutils.IsDir(t.out) {
		return fmt.Errorf("output directory '%v' does not exist", t.out)
	}

	type depConfig struct {
		name string
		path *string
	}
	dependencies := make([]depConfig, 0)
	switch t.taskMode {
	case TaskModeRun:
		if c.fuzzMode == FuzzModeWgsl {
			dependencies = append(dependencies, depConfig{"dictionary.txt", &t.dictionary})
		}
		fallthrough
	case TaskModeCheck:
		fuzzerName := "tint_wgsl_fuzzer"
		if c.fuzzMode == FuzzModeIr {
			fuzzerName = "tint_ir_fuzzer"
		}
		dependencies = append(dependencies, depConfig{fuzzerName, &t.fuzzer})
	case TaskModeGenerate:
		dependencies = append(dependencies, depConfig{"generate_tint_corpus.py", &t.generator})
		if c.fuzzMode == FuzzModeIr {
			dependencies = append(dependencies, depConfig{"ir_fuzz_as", &t.assembler})
		}
	}

	// Verify all the required dependencies are present
	for _, config := range dependencies {
		switch {
		case filepath.Ext(config.name) == ".py":
			*config.path = filepath.Join(filepath.Join(fileutils.DawnRoot(osWrapper), "src", "tint", "cmd", "fuzz"), config.name)
			if !fileutils.IsFile(*config.path) {
				return fmt.Errorf("script '%v' not found at '%v'", config.name, *config.path)
			}
		case filepath.Ext(config.name) == ".txt":
			*config.path = filepath.Join(filepath.Join(fileutils.DawnRoot(osWrapper), "src", "tint", "cmd", "fuzz", "wgsl"), config.name)
			if !fileutils.IsFile(*config.path) {
				return fmt.Errorf("resource '%v' not found at '%v'", config.name, *config.path)
			}
		default:
			*config.path = filepath.Join(t.build, config.name+fileutils.ExeExt)
			if !fileutils.IsExe(*config.path) {
				return fmt.Errorf("binary '%v' not found at '%v'", config.name, *config.path)
			}
		}
	}

	switch t.taskMode {
	case TaskModeRun:
		return runFuzzer(&t, osWrapper)
	case TaskModeCheck:
		return checkFuzzer(&t, osWrapper)
	case TaskModeGenerate:
		return runCorpusGenerator(&t, osWrapper)
	default:
		return fmt.Errorf("unknown task mode %d", t.taskMode)
	}
}

// TODO(crbug.com/344014313): Add unittests once term is converted to support
// dependency injection.
// checkFuzzer runs the fuzzer against all the test files the inputs directory,
// ensuring that the fuzzers do not error for the given file.
func checkFuzzer(t *taskConfig, osWrapper oswrapper.OSWrapper) error {
	var files []string
	if t.fuzzMode == FuzzModeIr {
		if t.inputs == "" {
			return fmt.Errorf("must explicitly provide a inputs when running in IR mode")
		}

		var err error
		files, err = glob.Glob(filepath.Join(t.inputs, "**.tirb"), osWrapper)
		if err != nil {
			return err
		}
	} else {
		var err error
		files, err = glob.Glob(filepath.Join(t.inputs, "**.wgsl"), osWrapper)
		if err != nil {
			return err
		}

		// Remove '*.expected.wgsl'
		files = transform.Filter(files, func(s string) bool { return !strings.Contains(s, ".expected.") })
	}
	fmt.Printf("checking %v files...\n", len(files))

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

	fmt.Println("done")
	return nil
}

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.
// runFuzzer runs the fuzzer across t.numProcesses processes.
// The fuzzer will use t.inputs as the seed directory.
// New cases are written to t.out.
// Blocks until a fuzzer errors, or the process is interrupted.
func runFuzzer(t *taskConfig, fsReader oswrapper.FilesystemReader) error {
	ctx := utils.CancelOnInterruptContext(context.Background())
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	args := generateFuzzerArgs(t, fsReader)

	if t.verbose {
		fmt.Println("Using fuzzing cmd: " + t.fuzzer + strings.Join(args, " "))
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

	fmt.Println("done")
	return nil
}

// TODO(crbug.com/344014313): Add unittests once fileutils is converted to use
// dependency injection.
// generateFuzzerArgs generates the arguments that need to be passed into the fuzzer binary call
func generateFuzzerArgs(t *taskConfig, fsReader oswrapper.FilesystemReader) []string {
	args := []string{t.out}

	if t.inputs != "" {
		args = append(args, t.inputs)
	}
	args = append(args, "-dict="+t.dictionary)
	if t.verbose {
		args = append(args, "--verbose")
	}
	if t.dump {
		args = append(args, "--dump")
	}
	if t.filter != "" {
		args = append(args, "--filter="+t.filter)
	}
	return args
}

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.
// runCorpusGenerator converts a set of input test files into a fuzzer corpus
// The fuzzer will use t.inputs as the source directory.
// The corpus will be written to t.out.
// Note: Currently execs to generate_tint_corpus.py
func runCorpusGenerator(t *taskConfig, fsReader oswrapper.FilesystemReader) error {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	args := generateGeneratorArgs(t, fsReader)
	if t.verbose {
		fmt.Println("Using generator cmd: " + t.generator + strings.Join(args, " "))
	}
	fmt.Println("running generator")

	cmd := exec.CommandContext(ctx, "python3", args...)
	out := bytes.Buffer{}
	cmd.Stdout = &out
	cmd.Stderr = &out
	if t.verbose {
		cmd.Stdout = io.MultiWriter(&out, os.Stdout)
		cmd.Stderr = io.MultiWriter(&out, os.Stderr)
	}

	if err := cmd.Run(); err != nil {
		return err
	}

	fmt.Println("done")
	return nil
}

// generateGeneratorArgs generates the arguments that need to be passed into python to run the corpus generator, this includes the script file
func generateGeneratorArgs(t *taskConfig, fsReader oswrapper.FilesystemReader) []string {
	args := []string{t.generator}
	if t.verbose {
		args = append(args, "--verbose")
	}
	if t.fuzzMode == FuzzModeIr {
		args = append(args, "--ir_as="+t.assembler)
	}
	args = append(args, t.inputs, t.out)

	return args
}

func defaultWgslCorpusDir(fsReader oswrapper.FilesystemReader) string {
	return filepath.Join(fileutils.DawnRoot(fsReader), "test", "tint")
}

func defaultBuildDir(fsReader oswrapper.FilesystemReader) string {
	return filepath.Join(fileutils.DawnRoot(fsReader), "out", "active")
}
