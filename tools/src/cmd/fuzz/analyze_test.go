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

	// Tint Core: src/tint (10 LF, 5 LH) + src/utils (20 LF, 15 LH) = 30 LF, 20 LH
	require.Equal(t, 30, cov.TintCore.LinesFound)
	require.Equal(t, 20, cov.TintCore.LinesHit)
	require.InDelta(t, 66.67, cov.TintCore.Percentage, 0.01)

	// Mesa: third_party/mesa (100 LF, 80 LH) = 100 LF, 80 LH
	require.Equal(t, 100, cov.Mesa.LinesFound)
	require.Equal(t, 80, cov.Mesa.LinesHit)
	require.InDelta(t, 80.0, cov.Mesa.Percentage, 0.01)

	// DirectX: third_party/directx-headers (50 LF, 10 LH) = 50 LF, 10 LH
	require.Equal(t, 50, cov.DirectX.LinesFound)
	require.Equal(t, 10, cov.DirectX.LinesHit)
	require.InDelta(t, 20.0, cov.DirectX.Percentage, 0.01)
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
			Fuzzer:         "fuzzerA",
			Corpus:         "corpusA",
			LimitType:      "seconds",
			LimitValue:     10,
			Iteration:      1,
			NormalizedSecs: 9.0,
			Coverage: IterationCoverage{
				TintCore: CoverageStats{
					LinesFound: 100,
					LinesHit:   50,
					Percentage: 50.0,
				},
			},
		},
		{
			Fuzzer:         "fuzzerA",
			Corpus:         "corpusA",
			LimitType:      "seconds",
			LimitValue:     10,
			Iteration:      2,
			NormalizedSecs: 11.0,
			Coverage: IterationCoverage{
				TintCore: CoverageStats{
					LinesFound: 100,
					LinesHit:   60,
					Percentage: 60.0,
				},
			},
		},
	}

	summaries := calculateStats(data)

	require.Len(t, summaries, 1)
	tKey := "fuzzerA - corpusA (Tint Core)"
	points, exists := summaries[tKey]
	require.True(t, exists)
	require.Len(t, points, 1)

	pt := points[0]
	require.Equal(t, "seconds", pt.LimitType)
	require.Equal(t, 10, pt.LimitValue)
	require.Equal(t, 2, pt.N)
	require.InDelta(t, 55.0, pt.CovAvg, 0.001)
	require.InDelta(t, 10.0, pt.NormSecsAvg, 0.001)
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
