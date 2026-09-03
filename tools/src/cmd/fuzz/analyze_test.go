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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestParseLcov(t *testing.T) {
	lcovContent := `
SF:/home/user/workspace/dawn/src/tint/lang/wgsl/parser/parser.cc
LF:10
LH:5
end_of_record
SF:../../src/utils/math.cc
LF:20
LH:15
end_of_record
SF:third_party/mesa/src/compiler/glsl_types.cpp
LF:100
LH:80
end_of_record
SF:third_party/directx-headers/dx.h
LF:50
LH:10
end_of_record
`
	cov := parseLcov(lcovContent)

	// Tint: src/tint (10 LF, 5 LH) + src/utils (20 LF, 15 LH) = 30 LF, 20 LH
	require.Equal(t, 30, cov[CoverageComponentTint].LinesFound)
	require.Equal(t, 20, cov[CoverageComponentTint].LinesHit)
	require.InDelta(t, 66.67, cov[CoverageComponentTint].Percentage, 0.01)

	// Mesa: third_party/mesa (100 LF, 80 LH) = 100 LF, 80 LH
	require.Equal(t, 100, cov[CoverageComponentMesa].LinesFound)
	require.Equal(t, 80, cov[CoverageComponentMesa].LinesHit)
	require.InDelta(t, 80.0, cov[CoverageComponentMesa].Percentage, 0.01)

	// DXC: third_party/directx-headers (50 LF, 10 LH) = 50 LF, 10 LH
	require.Equal(t, 50, cov[CoverageComponentDXC].LinesFound)
	require.Equal(t, 10, cov[CoverageComponentDXC].LinesHit)
	require.InDelta(t, 20.0, cov[CoverageComponentDXC].Percentage, 0.01)
}

func TestStats(t *testing.T) {
	vals := []float64{10.0, 20.0, 30.0}
	avg, stddev := computeAvgAndStdDev(vals)

	require.InDelta(t, 20.0, avg, 0.001)
	// Standard Deviation of [10, 20, 30] is 10
	require.InDelta(t, 10.0, stddev, 0.001)

	valsSingle := []float64{50.0}
	avgSingle, sdSingle := computeAvgAndStdDev(valsSingle)
	require.Equal(t, 50.0, avgSingle)
	require.Equal(t, 0.0, sdSingle)

	var valsEmpty []float64
	avgEmpty, sdEmpty := computeAvgAndStdDev(valsEmpty)
	require.Equal(t, 0.0, avgEmpty)
	require.Equal(t, 0.0, sdEmpty)
}

func TestCalculateStats(t *testing.T) {
	data := []IterationData{
		{
			Fuzzer:              "fuzzerA",
			Corpus:              "corpusA",
			LimitType:           "seconds",
			LimitValue:          10,
			Iteration:           1,
			NormalizationScore:  10.0,
			NormalizationError:  0.5,
			NormalizedSecs:      9.0,
			NormalizedSecsError: 0.45,
			Coverage: IterationCoverage{
				CoverageComponentTint: {
					LinesFound: 100,
					LinesHit:   50,
					Percentage: 50.0,
				},
			},
		},
		{
			Fuzzer:              "fuzzerA",
			Corpus:              "corpusA",
			LimitType:           "seconds",
			LimitValue:          10,
			Iteration:           2,
			NormalizationScore:  10.0,
			NormalizationError:  0.5,
			NormalizedSecs:      11.0,
			NormalizedSecsError: 0.55,
			Coverage: IterationCoverage{
				CoverageComponentTint: {
					LinesFound: 100,
					LinesHit:   60,
					Percentage: 60.0,
				},
			},
		},
	}

	summaries := calculateStats(data)

	require.Len(t, summaries, 1)
	pt := summaries[0]
	require.Equal(t, "fuzzerA", pt.Fuzzer)
	require.Equal(t, "corpusA", pt.Corpus)
	require.Equal(t, "Tint", pt.Component)
	require.Equal(t, "seconds", pt.LimitType)
	require.Equal(t, 10, pt.LimitValue)
	require.Equal(t, 2, pt.N)
	require.InDelta(t, 55.0, pt.CovAvg, 0.001)
	require.InDelta(t, 10.0, pt.NormSecsAvg, 0.001)
	// statErr = 1.0, sysErr = 0.5. Total = sqrt(1^2 + 0.5^2) = sqrt(1.25) ~ 1.11803
	require.InDelta(t, 1.118, pt.NormSecsSem, 0.001)
}

func TestMergeProfrawFiles_MissingTools(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()
	ew := execwrapper.NewTestExecWrapperForSuccess(nil, nil)

	cfg := &taskConfig{
		mainConfig: mainConfig{
			osWrapper:   fs,
			execWrapper: ew,
		},
	}

	err := mergeProfrawFiles(cfg, "/experiment/bin", []string{"a.profraw"}, "coverage.profdata")
	require.Error(t, err)
	require.Contains(t, err.Error(), "hermetic LLVM tool 'llvm-profdata' not found in experiment bin directory")
}

func TestMergeProfrawFiles_Success(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()
	_ = fs.MkdirAll("/experiment/bin", 0755)
	_ = fs.WriteFile("/experiment/bin/llvm-profdata", []byte("mock-profdata"), 0755)

	ew := execwrapper.NewTestExecWrapperForSuccess([]byte("merged output"), nil)

	cfg := &taskConfig{
		mainConfig: mainConfig{
			osWrapper:   fs,
			execWrapper: ew,
		},
	}

	err := mergeProfrawFiles(cfg, "/experiment/bin", []string{"/inputs/a.profraw"}, "/outputs/coverage.profdata")
	require.NoError(t, err)
}

func TestGenerateLcovReport_MissingTools(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()
	ew := execwrapper.NewTestExecWrapperForSuccess(nil, nil)

	cfg := &taskConfig{
		mainConfig: mainConfig{
			osWrapper:   fs,
			execWrapper: ew,
		},
	}

	err := generateLcovReport(cfg, "fuzzer", "/experiment/bin", "/outputs", "/inputs/coverage.profdata")
	require.Error(t, err)
	require.Contains(t, err.Error(), "hermetic LLVM tool 'llvm-cov' not found in experiment bin directory")
}

func TestGenerateLcovReport_Success(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()
	_ = fs.MkdirAll("/experiment/bin", 0755)
	_ = fs.WriteFile("/experiment/bin/llvm-cov", []byte("mock-cov"), 0755)

	ew := execwrapper.NewTestExecWrapperForSuccess([]byte("report generated"), nil)

	cfg := &taskConfig{
		mainConfig: mainConfig{
			osWrapper:   fs,
			execWrapper: ew,
		},
	}

	err := generateLcovReport(cfg, "fuzzer", "/experiment/bin", "/outputs", "/inputs/coverage.profdata")
	require.NoError(t, err)
}

func TestPrintStatsCSVAndReport(t *testing.T) {
	fs := oswrapper.CreateFSTestOSWrapper()
	_ = fs.MkdirAll("/experiment", 0755)

	ac := &analyzeConfig{
		taskConfig: &taskConfig{
			mainConfig: mainConfig{
				osWrapper:   fs,
				analyzePath: "/experiment",
			},
		},
		settings: ExperimentSettings{
			Name: "test_experiment",
			Hash: "abcdef123",
		},
		resultsDir: "/experiment/results",
		machines:   []string{"machineA"},
	}

	stats := []SummaryPoint{
		{
			Fuzzer:       "fuzzerA",
			Corpus:       "corpusA",
			Component:    "Tint",
			LimitType:    "seconds",
			LimitValue:   10,
			NormSecsAvg:  9.5,
			NormSecsSem:  0.5,
			CovAvg:       55.0,
			CovSem:       1.2,
			CovRate:      5.78,
			CovRateError: 0.1,
			N:            5,
		},
		{
			Fuzzer:       "fuzzerA",
			Corpus:       "corpusA",
			Component:    "Tint",
			LimitType:    "seconds",
			LimitValue:   20,
			NormSecsAvg:  19.5,
			NormSecsSem:  0.8,
			CovAvg:       65.0,
			CovSem:       1.5,
			CovRate:      3.33,
			CovRateError: 0.12,
			N:            5,
		},
	}

	// 1. Test printStatsCSV
	err := ac.printStatsCSV(stats)
	require.NoError(t, err)

	csvContent, err := fs.ReadFile("/experiment/calculated_statistics.csv")
	require.NoError(t, err)

	expectedCSV := "Fuzzer,Corpus,Component,Samples,LimitType,LimitValue,NormalizedCPUSecondsAvg,NormalizedCPUSecondsSEM,CoveragePercentAvg,CoveragePercentSEM,CoverageRateAvg,CoverageRateSEM\n" +
		"fuzzerA,corpusA,Tint,5,seconds,10,9.5000,0.5000,55.00,1.20,5.780000,0.100000\n" +
		"fuzzerA,corpusA,Tint,5,seconds,20,19.5000,0.8000,65.00,1.50,3.330000,0.120000\n"

	require.Equal(t, expectedCSV, string(csvContent))

	// 2. Test printReport
	err = ac.printReport(stats)
	require.NoError(t, err)

	reportContent, err := fs.ReadFile("/experiment/experiment_report.md")
	require.NoError(t, err)

	reportStr := string(reportContent)
	require.Contains(t, reportStr, "# Experiment Performance and Coverage Report: test_experiment")
	require.Contains(t, reportStr, "- **Git Hash**: `abcdef123`")
	require.Contains(t, reportStr, "### fuzzerA - corpusA (Tint)")
	require.Contains(t, reportStr, "| Samples (N) | Target Limit | Normalized CPU Seconds (Avg ± SEM) | Coverage % (Avg ± SEM) | Coverage Rate (%/sec) (Avg ± SEM) |")
	require.Contains(t, reportStr, "| 5           | 10 seconds   | 9.50 ± 0.50                        | 55.00% ± 1.20%         | 5.780000 ± 0.100000               |")
	require.Contains(t, reportStr, "| 5           | 20 seconds   | 19.50 ± 0.80                       | 65.00% ± 1.50%         | 3.330000 ± 0.120000               |")
}
