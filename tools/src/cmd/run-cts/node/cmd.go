// Copyright 2023 The Dawn & Tint Authors
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

package node

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"runtime"
	"strings"
	"syscall"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/cov"
	"dawn.googlesource.com/dawn/tools/src/dawn/node"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// runnerName is the file name of the standalone runner executable, without extension.
const runnerName = "dawn_standalone_cts_runner"

type flags struct {
	common.Flags
	bin                  string
	runner               string
	backend              string
	adapterName          string
	coverageFile         string
	isolated             bool
	build                bool
	validate             bool
	dumpShaders          bool
	dumpShadersPretty    bool
	fxc                  bool
	unrollConstEvalLoops bool
	genCoverage          bool
	compatibilityMode    bool
	debugCTS             bool
	skipVSCodeInfo       bool
	enforceDefaultLimits bool
	blockAllFeatures     bool
	batchSize            int
	caseTimeout          time.Duration
	startupAllowance     time.Duration
	recover              bool
	dawn                 node.Flags
}

type defaultCmd struct {
	cmd
}

func (*defaultCmd) IsDefaultCommand() {}

func init() {
	common.Register(&defaultCmd{})
	common.Register(&cmd{standalone: true})
}

type cmd struct {
	standalone bool
	state      common.State
	flags      flags
	coverage   *common.Coverage
	queries    []string
}

func (c *cmd) Name() string {
	if c.standalone {
		return "standalone"
	}
	return "node"
}

func (c *cmd) Desc() string {
	if c.standalone {
		return "runs the CTS with " + runnerName
	}
	return "runs the CTS with dawn.node"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	unrollConstEvalLoopsDefault := false

	backendDefault := "default"
	if vkIcdFilenames := cfg.OsWrapper.Getenv("VK_ICD_FILENAMES"); vkIcdFilenames != "" {
		backendDefault = "vulkan"
	}

	binDesc := "path to the directory holding cts.cjs and dawn.node"
	if c.standalone {
		binDesc = "path to the directory holding " + runnerName
	}

	c.flags.Flags.Register(cfg.OsWrapper)
	flag.StringVar(&c.flags.bin, "bin", fileutils.BuildPath(cfg.OsWrapper), binDesc)
	if c.standalone {
		flag.StringVar(&c.flags.runner, "runner", "", "path to "+runnerName+", overriding -bin")
		flag.IntVar(&c.flags.batchSize, "batch-size", defaultBatchSize, "maximum number of test cases per runner process")
	} else {
		flag.BoolVar(&c.flags.isolated, "isolate", false, "run each test in an isolated process")
	}
	flag.BoolVar(&c.flags.build, "build", true, "attempt to build the CTS before running")
	flag.BoolVar(&c.flags.validate, "validate", false, "enable backend validation")
	flag.Var(&c.flags.dawn, "flag", "flag to pass to dawn-node as flag=value. multiple flags must be passed in individually")
	flag.StringVar(&c.flags.backend, "backend", backendDefault, "backend to use: default|null|webgpu|d3d11|d3d12|metal|vulkan|opengl|opengles."+
		" set to 'vulkan' if VK_ICD_FILENAMES environment variable is set, 'default' otherwise")
	flag.StringVar(&c.flags.adapterName, "adapter", "", "name (or substring) of the GPU adapter to use")
	flag.BoolVar(&c.flags.dumpShaders, "dump-shaders", false, "dump WGSL shaders. Enables --verbose")
	flag.BoolVar(&c.flags.dumpShadersPretty, "dump-shaders-pretty", false, "dump WGSL shaders, but don't run symbol renaming. May fail tests that shadow predeclared builtins. Enables --verbose")
	flag.BoolVar(&c.flags.fxc, "fxc", false, "Use FXC instead of DXC. Disables 'use_dxc' Dawn flag")
	flag.BoolVar(&c.flags.unrollConstEvalLoops, "unroll-const-eval-loops", unrollConstEvalLoopsDefault, "unroll loops in const-eval tests")
	if !c.standalone {
		flag.BoolVar(&c.flags.genCoverage, "coverage", false, "displays coverage data")
		flag.StringVar(&c.flags.coverageFile, "export-coverage", "", "write coverage data to the given path")
	}
	flag.BoolVar(&c.flags.compatibilityMode, "compat", false, "run tests in compatibility mode")
	flag.BoolVar(&c.flags.debugCTS, "debug-cts", false, "enable CTS debugging option")
	flag.BoolVar(&c.flags.skipVSCodeInfo, "skip-vs-code-info", false, "skips emitting VS Code information")
	flag.BoolVar(&c.flags.enforceDefaultLimits, "enforce-default-limits", false, "enforce the default limits (note: powerPreference tests may fail)")
	flag.BoolVar(&c.flags.blockAllFeatures, "block-all-features", false, "block all features (except 'core-features-and-limits')")
	flag.DurationVar(&c.flags.caseTimeout, "case-timeout", common.TestCaseTimeout, "kill a runner process if it produces no result line within this time")
	flag.DurationVar(&c.flags.startupAllowance, "startup-allowance", defaultStartupAllowance, "extra time allowed for the first result line of each runner process")
	flag.BoolVar(&c.flags.recover, "recover", true, "start a new runner process on the remaining test cases after a crash or timeout")

	if c.standalone {
		return nil, nil
	}
	return []string{"[query]"}, nil
}

// TODO(crbug.com/416755658): Add unittest coverage when there is a way to fake
// the node process.
func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Process the command line flags
	if err := c.processFlags(cfg.OsWrapper); err != nil {
		return err
	}

	if err := c.maybeInitCoverage(cfg.OsWrapper); err != nil {
		return err
	}

	ctx, stop := signal.NotifyContext(ctx, os.Interrupt, syscall.SIGTERM)
	defer stop()

	if c.flags.build {
		if err := c.state.CTS.Node.BuildIfRequired(c.flags.Verbose, cfg.OsWrapper); err != nil {
			return err
		}
	}

	testCases, err := c.queryTestCases()
	if err != nil {
		return err
	}
	fmt.Printf("Testing %d test cases...\n", len(testCases))

	var runner func(ctx context.Context, testCases []common.TestCase, results chan<- common.Result, fsReaderWriter oswrapper.FilesystemReaderWriter)
	if c.standalone || c.flags.isolated {
		fmt.Println("Running in parallel isolated...")
		runner = c.runTestCasesWithCmdline
	} else {
		fmt.Println("Running in parallel with server...")
		runner = c.runTestCasesWithServers
	}

	resultStream := make(chan common.Result, 256)
	go func() {
		runner(ctx, testCases, resultStream, cfg.OsWrapper)
		close(resultStream)
	}()

	results, err := common.StreamResults(ctx,
		c.flags.Colors,
		c.flags.FailuresOnly,
		c.state,
		c.flags.Verbose,
		c.coverage,
		len(testCases),
		resultStream,
		cfg.OsWrapper)

	// Make sure we always save results, even if there were failures.
	if results != nil {
		if err := c.state.Close(results); err != nil {
			return err
		}
	}

	if err != nil {
		return err
	}

	return nil
}

// target returns where to run the command-line runner, and the CTS directory there.
func (c *cmd) target() (localTarget, string) {
	runner := c.flags.Node
	if c.standalone {
		runner = c.flags.runner
	}
	return localTarget{
		runner:  runner,
		dir:     c.flags.CTS,
		verbose: c.flags.Verbose,
		// The VS Code launch.json output does not work for the standalone runner,
		// which reads its test cases from stdin.
		skipVSCodeInfo: c.flags.skipVSCodeInfo || c.standalone,
	}, c.flags.CTS
}

func (c *cmd) processFlags(fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	if c.standalone {
		if c.flags.runner == "" {
			if c.flags.bin == "" {
				return fmt.Errorf("-bin is not set. It defaults to <dawn>/out/active (%v) which does not exist",
					filepath.Join(fileutils.DawnRoot(fsReaderWriter), "out/active"))
			}
			c.flags.runner = filepath.Join(c.flags.bin, runnerName+fileutils.ExeExt)
		}
		if !fileutils.IsFile(c.flags.runner, fsReaderWriter) {
			return fmt.Errorf("'%v' does not exist", c.flags.runner)
		}
		absRunner, err := filepath.Abs(c.flags.runner)
		if err != nil {
			return fmt.Errorf("unable to get absolute path for '%v'", c.flags.runner)
		}
		c.flags.runner = absRunner
		if c.flags.batchSize < 1 {
			return fmt.Errorf("-batch-size must be at least 1")
		}
	} else {
		if c.flags.bin == "" {
			return fmt.Errorf("-bin is not set. It defaults to <dawn>/out/active (%v) which does not exist",
				filepath.Join(fileutils.DawnRoot(fsReaderWriter), "out/active"))
		}
		if !fileutils.IsDir(c.flags.bin, fsReaderWriter) {
			return fmt.Errorf("'%v' is not a directory", c.flags.bin)
		}
		for _, file := range []string{"cts.cjs", "dawn.node"} {
			if !fileutils.IsFile(filepath.Join(c.flags.bin, file), fsReaderWriter) {
				return fmt.Errorf("'%v' does not contain '%v'", c.flags.bin, file)
			}
		}
		absBin, err := filepath.Abs(c.flags.bin)
		if err != nil {
			return fmt.Errorf("unable to get absolute path for '%v'", c.flags.bin)
		}
		c.flags.bin = absBin
	}

	c.queries = flag.Args()
	if len(c.queries) == 0 {
		c.queries = []string{"webgpu:*"}
	}
	// The flag package stops parsing at the first query, so a flag given after
	// it would otherwise be taken for a query.
	for _, query := range c.queries {
		if strings.HasPrefix(query, "-") {
			return fmt.Errorf("'%v' is not a query; flags must come before the queries", query)
		}
	}
	if !c.standalone && !c.flags.isolated && len(c.queries) > 1 {
		return fmt.Errorf("only a single query can be provided")
	}
	if c.flags.NumRunners < 1 {
		c.flags.NumRunners = 1
	}
	if c.flags.caseTimeout <= 0 {
		return fmt.Errorf("-case-timeout must be positive")
	}
	if c.flags.startupAllowance < 0 {
		return fmt.Errorf("-startup-allowance must not be negative")
	}

	c.flags.dawn.SetOptions(node.Options{
		Backend:           c.flags.backend,
		Adapter:           c.flags.adapterName,
		Validate:          c.flags.validate,
		AllowUnsafeAPIs:   true,
		DumpShaders:       c.flags.dumpShaders,
		DumpShadersPretty: c.flags.dumpShadersPretty,
		UseFXC:            c.flags.fxc,
	})

	if c.flags.dumpShaders {
		c.flags.Verbose = true
	}
	if c.flags.dumpShadersPretty {
		c.flags.Verbose = true
	}

	state, err := c.flags.Process(fsReaderWriter)
	if err != nil {
		return err
	}
	c.state = *state

	return nil
}

// queryTestCases returns the test cases that match c.queries, in order.
func (c *cmd) queryTestCases() ([]common.TestCase, error) {
	testCases := []common.TestCase{}
	queryOf := map[common.TestCase]string{}
	for _, query := range c.queries {
		cases, err := c.state.CTS.QueryTestCases(c.flags.Verbose, query)
		if err != nil {
			return nil, fmt.Errorf("failed to gather tests: %w", err)
		}
		for _, testCase := range cases {
			if other, found := queryOf[testCase]; found {
				return nil, fmt.Errorf("'%v' matches both '%v' and '%v'", testCase, other, query)
			}
			queryOf[testCase] = query
		}
		testCases = append(testCases, cases...)
	}
	return testCases, nil
}

// TODO(crbug.com/416755658): Add unittest coverage when exec is handled via
// dependency injection.
func (c *cmd) maybeInitCoverage(fsReader oswrapper.FilesystemReader) error {
	if !c.flags.genCoverage && c.flags.coverageFile == "" {
		return nil
	}

	profdata, err := exec.LookPath("llvm-profdata")
	if err != nil {
		profdata = ""
		if runtime.GOOS == "darwin" {
			profdata = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-profdata"
			if !fileutils.IsExe(profdata, fsReader) {
				profdata = ""
			}
		}
	}
	if profdata == "" {
		return fmt.Errorf("failed to find llvm-profdata, required for --coverage")
	}

	llvmCov := ""
	turboCov := filepath.Join(c.flags.bin, "turbo-cov"+fileutils.ExeExt)
	if !fileutils.IsExe(turboCov, fsReader) {
		turboCov = ""
		if path, err := exec.LookPath("llvm-cov"); err == nil {
			llvmCov = path
		} else {
			return fmt.Errorf("failed to find turbo-cov or llvm-cov")
		}
	}
	c.coverage = &common.Coverage{
		OutputFile: c.flags.coverageFile,
		Env: &cov.Env{
			Profdata: profdata,
			Binary:   filepath.Join(c.flags.bin, "dawn.node"),
			Cov:      llvmCov,
			TurboCov: turboCov,
		},
	}

	return nil
}
