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

package main

import (
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestCheckGnArgs(t *testing.T) {
	tests := []struct {
		name    string
		gnJson  string
		fuzzers []string
		wantErr bool
	}{
		{
			name: "Correct baseline flags",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}}
			]`,
			fuzzers: []string{"tint_wgsl_fuzzer"},
			wantErr: false,
		},
		{
			name: "Missing use_clang_coverage",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}}
			]`,
			fuzzers: []string{"tint_wgsl_fuzzer"},
			wantErr: true,
		},
		{
			name: "optimize_for_fuzzing is true",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "true"}}
			]`,
			fuzzers: []string{"tint_wgsl_fuzzer"},
			wantErr: true,
		},
		{
			name: "Correct IR flags",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}},
				{"name": "tint_build_ir_binary", "current": {"value": "true"}},
				{"name": "tint_has_protobuf", "current": {"value": "true"}}
			]`,
			fuzzers: []string{"tint_ir_fuzzer"},
			wantErr: false,
		},
		{
			name: "Missing IR protobuf flag",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}},
				{"name": "tint_build_ir_binary", "current": {"value": "true"}}
			]`,
			fuzzers: []string{"tint_ir_fuzzer"},
			wantErr: true,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			fs := oswrapper.CreateFSTestOSWrapper()
			ew := execwrapper.NewTestExecWrapperForSuccess([]byte(tc.gnJson), nil)

			cfg := &taskConfig{
				mainConfig: mainConfig{
					build:       "/build",
					osWrapper:   fs,
					execWrapper: ew,
				},
			}

			_, err := checkGnArgs(cfg, tc.fuzzers)
			if tc.wantErr {
				require.Error(t, err)
			} else {
				require.NoError(t, err)
			}
		})
	}
}

func TestGenerateTasksForFuzzer(t *testing.T) {
	secs := 3600
	runs := 100000
	customIters := 2

	corpus := CorpusDef{Name: "tint_tests", Path: "tint_tests_dir"}

	settings := &ExperimentSettings{
		DefaultIterations: 5,
		Durations: []DurationDef{
			{Seconds: &secs},
			{Runs: &runs, Iterations: &customIters},
		},
	}

	tasks, err := calculateExperimentTasksForFuzzer("tint_wgsl_fuzzer", corpus, "/root/corpora", "/root/results/default", settings)

	require.NoError(t, err)
	require.Len(t, tasks, 7)

	require.Equal(t, "tint_wgsl_fuzzer", tasks[0].FuzzerName)
	require.Equal(t, "tint_tests", tasks[0].CorpusName)
	require.Equal(t, filepath.Join("/root/corpora", "tint_tests_dir"), tasks[0].CorpusPath)
	require.Equal(t, ExperimentLimitTypeSeconds, tasks[0].LimitType)
	require.Equal(t, 3600, tasks[0].LimitValue)
	require.Equal(t, 1, tasks[0].Iteration)
	require.Equal(t, filepath.Join("/root/results/default", "tint_wgsl_fuzzer", "tint_tests", "seconds_3600", "iter_1"), tasks[0].TaskDir)

	lastIdx := len(tasks) - 1
	require.Equal(t, ExperimentLimitTypeRuns, tasks[lastIdx].LimitType)
	require.Equal(t, 100000, tasks[lastIdx].LimitValue)
	require.Equal(t, 2, tasks[lastIdx].Iteration)
	require.Equal(t, filepath.Join("/root/results/default", "tint_wgsl_fuzzer", "tint_tests", "runs_100000", "iter_2"), tasks[lastIdx].TaskDir)
}

func TestLoadExperimentSettings(t *testing.T) {
	tests := []struct {
		name        string
		setupFS     func(fs oswrapper.FSTestOSWrapper)
		wantErr     bool
		errContains string
		validate    func(t *testing.T, settings ExperimentSettings)
	}{
		{
			name: "Valid experiment config",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10},
						{"runs": 1000}
					]
				}`), 0644)
			},
			wantErr: false,
			validate: func(t *testing.T, settings ExperimentSettings) {
				require.Nil(t, settings.BenchmarkDuration)
			},
		},
		{
			name: "Valid experiment config with benchmark duration",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10},
						{"runs": 1000}
					],
					"benchmark_duration": 45
				}`), 0644)
			},
			wantErr: false,
			validate: func(t *testing.T, settings ExperimentSettings) {
				require.NotNil(t, settings.BenchmarkDuration)
				require.Equal(t, 45, *settings.BenchmarkDuration)
			},
		},
		{
			name: "Valid experiment config with burn-in duration and enabled",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					],
					"burnin_duration": 120,
					"burnin_enabled": false
				}`), 0644)
			},
			wantErr: false,
			validate: func(t *testing.T, config ExperimentSettings) {
				require.NotNil(t, config.BurnInDuration)
				require.Equal(t, 120, *config.BurnInDuration)
				require.NotNil(t, config.BurnInEnabled)
				require.False(t, *config.BurnInEnabled)
			},
		},
		{
			name: "Invalid experiment config (both limit types set)",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10, "runs": 1000}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "has both 'seconds' and 'runs' defined",
		},
		{
			name: "Invalid experiment config (neither limit type set)",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"iterations": 5}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "must define either 'seconds' or 'runs'",
		},
		{
			name: "Missing config file",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
			},
			wantErr:     true,
			errContains: "experiment configuration file '/root/experiment.json' not found",
		},
		{
			name: "No fuzzers specified",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": [],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "no fuzzers specified in 'fuzzers' list",
		},
		{
			name: "Unsupported fuzzer",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["invalid_fuzzer_name"],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "unsupported fuzzer 'invalid_fuzzer_name'",
		},
		{
			name: "Missing WGSL benchmark corpus",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "wgsl_benchmark_corpus is required in experiment.json because WGSL fuzzers are specified",
		},
		{
			name: "Missing WGSL corpora",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "at least one wgsl_corpora definition is required in experiment.json because WGSL fuzzers are specified",
		},
		{
			name: "WGSL benchmark corpus directory missing",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "wgsl benchmark corpus directory 'bench' not found under corpora root",
		},
		{
			name: "WGSL corpus directory missing",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "wgsl corpus directory 'test_corp' not found under corpora root",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			fs := oswrapper.CreateFSTestOSWrapper()
			tc.setupFS(fs)

			cfg := &taskConfig{
				mainConfig: mainConfig{
					osWrapper: fs,
				},
			}

			settings, err := loadExperimentSettings(cfg, "/root")
			if tc.wantErr {
				require.Error(t, err)
				require.Contains(t, err.Error(), tc.errContains)
			} else {
				require.NoError(t, err)
				require.Equal(t, "test_exp", settings.Name)
				if tc.validate != nil {
					tc.validate(t, settings)
				}
			}
		})
	}
}

func TestPrepareBinariesWithRuntimeDeps(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()

	// Create a test build directory and the fuzzer binary
	_ = fs.MkdirAll("/build", 0755)
	_ = fs.WriteFile("/build/tint_wgsl_fuzzer", []byte("fuzzer-binary"), 0755)

	// Create mock LLVM tools
	_ = fs.MkdirAll("third_party/llvm-build/Release+Asserts/bin", 0755)
	_ = fs.WriteFile("third_party/llvm-build/Release+Asserts/bin/llvm-profdata", []byte("mock-profdata"), 0755)
	_ = fs.WriteFile("third_party/llvm-build/Release+Asserts/bin/llvm-cov", []byte("mock-cov"), 0755)

	// Create runtime_deps file
	depsContent := "tint_wgsl_fuzzer\nlib/libswiftshader.so\nlibvulkan.so.1\nsrc/some_other_dep.dat\nlvp_icd.json\n"
	_ = fs.WriteFile("/build/tint_wgsl_fuzzer.runtime_deps", []byte(depsContent), 0644)

	// Create test runtime dependencies to copy
	_ = fs.MkdirAll("/build/lib", 0755)
	_ = fs.WriteFile("/build/lib/libswiftshader.so", []byte("swiftshader-binary"), 0755)
	_ = fs.WriteFile("/build/libvulkan.so.1", []byte("vulkan-binary"), 0755)
	_ = fs.WriteFile("/build/lvp_icd.json", []byte("icd-json"), 0644)
	_ = fs.MkdirAll("/build/src", 0755)
	_ = fs.WriteFile("/build/src/some_other_dep.dat", []byte("some-data"), 0644)

	ew := execwrapper.NewTestExecWrapperForSuccess([]byte("main\n"), nil)

	cfg := &taskConfig{
		mainConfig: mainConfig{
			build:       "/build",
			osWrapper:   fs,
			execWrapper: ew,
		},
	}

	settings := &ExperimentSettings{
		Hash: "mock-hash",
	}

	binDir := "/experiment/bin"
	err := prepareBinaries(cfg, settings, binDir, []string{"tint_wgsl_fuzzer"})
	require.NoError(t, err)

	// Verify binary was copied
	require.True(t, fileutils.IsFile("/experiment/bin/tint_wgsl_fuzzer", fs))
	contentBin, _ := fs.ReadFile("/experiment/bin/tint_wgsl_fuzzer")
	require.Equal(t, "fuzzer-binary", string(contentBin))

	// Verify runtime dependencies were copied
	require.True(t, fileutils.IsFile("/experiment/bin/lib/libswiftshader.so", fs))
	contentLib, _ := fs.ReadFile("/experiment/bin/lib/libswiftshader.so")
	require.Equal(t, "swiftshader-binary", string(contentLib))

	// Verify versioned runtime dependencies were copied
	require.True(t, fileutils.IsFile("/experiment/bin/libvulkan.so.1", fs))
	contentVul, _ := fs.ReadFile("/experiment/bin/libvulkan.so.1")
	require.Equal(t, "vulkan-binary", string(contentVul))

	// Verify lvp_icd.json was copied
	require.True(t, fileutils.IsFile("/experiment/bin/lvp_icd.json", fs))
	contentIcd, _ := fs.ReadFile("/experiment/bin/lvp_icd.json")
	require.Equal(t, "icd-json", string(contentIcd))

	// Verify non-library dependencies were NOT copied
	require.False(t, fileutils.IsFile("/experiment/bin/src/some_other_dep.dat", fs))

	// Verify LLVM tools were copied
	require.True(t, fileutils.IsFile("/experiment/bin/llvm-profdata", fs))
	contentProfdata, _ := fs.ReadFile("/experiment/bin/llvm-profdata")
	require.Equal(t, "mock-profdata", string(contentProfdata))

	require.True(t, fileutils.IsFile("/experiment/bin/llvm-cov", fs))
	contentCov, _ := fs.ReadFile("/experiment/bin/llvm-cov")
	require.Equal(t, "mock-cov", string(contentCov))
}

func TestPrepareBinariesNoRuntimeDeps(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()

	// Create a test build directory and the fuzzer binary
	_ = fs.MkdirAll("/build", 0755)
	_ = fs.WriteFile("/build/tint_wgsl_fuzzer", []byte("fuzzer-binary"), 0755)

	// Create mock LLVM tools
	_ = fs.MkdirAll("third_party/llvm-build/Release+Asserts/bin", 0755)
	_ = fs.WriteFile("third_party/llvm-build/Release+Asserts/bin/llvm-profdata", []byte("mock-profdata"), 0755)
	_ = fs.WriteFile("third_party/llvm-build/Release+Asserts/bin/llvm-cov", []byte("mock-cov"), 0755)

	ew := execwrapper.NewTestExecWrapperForSuccess([]byte("main\n"), nil)

	cfg := &taskConfig{
		mainConfig: mainConfig{
			build:       "/build",
			osWrapper:   fs,
			execWrapper: ew,
		},
	}

	settings := &ExperimentSettings{
		Hash: "mock-hash",
	}

	binDir := "/experiment/bin"
	err := prepareBinaries(cfg, settings, binDir, []string{"tint_wgsl_fuzzer"})
	require.NoError(t, err)

	// Verify fuzzer binary was copied
	require.True(t, fileutils.IsFile("/experiment/bin/tint_wgsl_fuzzer", fs))
	contentBin, _ := fs.ReadFile("/experiment/bin/tint_wgsl_fuzzer")
	require.Equal(t, "fuzzer-binary", string(contentBin))

	// Verify LLVM tools were copied
	require.True(t, fileutils.IsFile("/experiment/bin/llvm-profdata", fs))
	contentProfdata, _ := fs.ReadFile("/experiment/bin/llvm-profdata")
	require.Equal(t, "mock-profdata", string(contentProfdata))

	require.True(t, fileutils.IsFile("/experiment/bin/llvm-cov", fs))
	contentCov, _ := fs.ReadFile("/experiment/bin/llvm-cov")
	require.Equal(t, "mock-cov", string(contentCov))
}

func TestAppendLibraryArgs(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()

	// 1. Empty bin directory
	_ = fs.MkdirAll("/experiment/bin", 0755)
	args := []string{"arg1", "arg2"}
	args = appendLibraryArgs(args, "/experiment/bin", fs)
	require.Equal(t, []string{"arg1", "arg2"}, args)

	// 2. With DXC and ICD files in bin root
	_ = fs.WriteFile("/experiment/bin/libdxcompiler.so", []byte("dxc"), 0755)
	_ = fs.WriteFile("/experiment/bin/lvp_icd.json", []byte("icd"), 0644)
	_ = fs.WriteFile("/experiment/bin/vk_swiftshader_icd.json", []byte("swiftshader-icd"), 0644)

	args = []string{"arg1", "arg2"}
	args = appendLibraryArgs(args, "/experiment/bin", fs)
	require.Contains(t, args, "--dxc=/experiment/bin/libdxcompiler.so")
	require.Contains(t, args, "--vk_icd=/experiment/bin/lvp_icd.json")
	require.NotContains(t, args, "--vk_icd=/experiment/bin/vk_swiftshader_icd.json")

	// 3. With DXC and ICD files in subdirectories (nested)
	fs2 := oswrapper.CreateFSTestOSWrapper()
	_ = fs2.MkdirAll("/experiment/bin/lib", 0755)
	_ = fs2.MkdirAll("/experiment/bin/config", 0755)
	_ = fs2.WriteFile("/experiment/bin/lib/libdxcompiler.so", []byte("dxc"), 0755)
	_ = fs2.WriteFile("/experiment/bin/config/lvp_icd.json", []byte("icd"), 0644)
	_ = fs2.WriteFile("/experiment/bin/config/vk_swiftshader_icd.json", []byte("swiftshader-icd"), 0644)

	args2 := []string{"arg1", "arg2"}
	args2 = appendLibraryArgs(args2, "/experiment/bin", fs2)
	require.Contains(t, args2, "--dxc=/experiment/bin/lib/libdxcompiler.so")
	require.Contains(t, args2, "--vk_icd=/experiment/bin/config/lvp_icd.json")
	require.NotContains(t, args2, "--vk_icd=/experiment/bin/config/vk_swiftshader_icd.json")
}
