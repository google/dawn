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
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"math"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"sync"
	"unicode/utf8"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// Analyze mode path stems that get used multiple times
const (
	kAnalyzeResultsDir   = "results"
	kAnalyzeReportFile   = "experiment_report.md"
	kAnalyzeCsvFile      = "raw_iteration_data.csv"
	kAnalyzeStatsCsvFile = "calculated_statistics.csv"
)

// CoverageStats holds the line coverage metrics for a specific code component.
type CoverageStats struct {
	LinesFound int     `json:"lines_found"`
	LinesHit   int     `json:"lines_hit"`
	Percentage float64 `json:"percentage"`
}

// IterationCoverage aggregates coverage statistics across different project components.
type IterationCoverage struct {
	TintCore CoverageStats `json:"tint_core"`
	Mesa     CoverageStats `json:"mesa"`
	DirectX  CoverageStats `json:"directx"`
}

// IterationData holds the raw performance and coverage metrics for a single fuzzer iteration.
type IterationData struct {
	Machine        string
	Fuzzer         string
	Corpus         string
	LimitType      string
	LimitValue     int
	Iteration      int
	PerfScore      float64
	ActualSeconds  float64
	ActualRuns     int
	NormalizedSecs float64
	Coverage       IterationCoverage
}

// SummaryPoint represents a single statistical data point in a fuzzer's
// coverage trajectory, summarizing metrics across multiple iterations.
type SummaryPoint struct {
	Fuzzer       string
	Corpus       string
	Component    string
	LimitType    string
	LimitValue   int
	NormSecsAvg  float64
	NormSecsSem  float64
	CovAvg       float64
	CovSem       float64
	CovRate      float64
	CovRateError float64
	N            int
}

// runAnalyze entry point for running the analysis task.
func runAnalyze(t *taskConfig) error {
	settings, err := loadExperimentSettings(t, t.analyzePath)
	if err != nil {
		return err
	}

	resultsDir := filepath.Join(t.analyzePath, kAnalyzeResultsDir)
	if !fileutils.IsDir(resultsDir, t.osWrapper) {
		return fmt.Errorf("results directory '%s' not found", resultsDir)
	}

	ac := &analyzeConfig{
		taskConfig: t,
		settings:   settings,
		resultsDir: resultsDir,
		data:       make([]IterationData, 0),
	}
	return ac.run()
}

// analyzeConfig holds the configuration and state for the analysis task.
type analyzeConfig struct {
	*taskConfig
	settings   ExperimentSettings
	resultsDir string
	machines   []string
	data       []IterationData
}

// run generates a Markdown performance and coverage report by analyzing
// the results of completed fuzzing iterations stored in the report path. Also
// emits full/raw stats to a .csv file for processing by other tools.
func (ac *analyzeConfig) run() error {
	if err := ac.findMachines(); err != nil {
		return err
	}

	err := ac.gatherData()
	if err != nil {
		return err
	}

	if err := ac.printRawCSV(); err != nil {
		return err
	}

	stats := calculateStats(ac.data)
	if err := ac.printStatsCSV(stats); err != nil {
		return err
	}

	if err := ac.printReport(stats); err != nil {
		return err
	}

	return nil
}

// findMachines scans the results directory to identify and populate the list of machine names to analyze,
// filtering by a specific machine if configured. Returns an error if no machine result can be found.
func (ac *analyzeConfig) findMachines() error {
	var machines []string
	if ac.machineName != "" {
		mPath := filepath.Join(ac.resultsDir, ac.machineName)
		if !fileutils.IsDir(mPath, ac.osWrapper) {
			return fmt.Errorf("specified machine results directory '%s' not found", mPath)
		}
		machines = append(machines, ac.machineName)
	} else {
		// Scan results Dir
		files, err := ac.osWrapper.ReadDir(ac.resultsDir)
		if err != nil {
			return fmt.Errorf("failed to read results directory: %w", err)
		}
		for _, f := range files {
			if f.IsDir() {
				machines = append(machines, f.Name())
			}
		}
	}

	if len(machines) == 0 {
		return fmt.Errorf("no machine results found under '%s'", ac.resultsDir)
	}

	ac.machines = machines
	fmt.Printf("Analyzing results from machines: %s\n", strings.Join(ac.machines, ", "))
	return nil
}

// gatherData scans the results directory for each machine, loads performance
// scores, and generates a list of iteration data for analysis.
func (ac *analyzeConfig) gatherData() error {
	for _, machine := range ac.machines {
		machineData, err := ac.calculateMachineDataTasks(machine)
		if err != nil {
			return err
		}
		ac.data = append(ac.data, machineData...)
	}

	if len(ac.data) == 0 {
		return fmt.Errorf("no completed iterations found to analyze")
	}

	err := ac.runDataTasks()
	if err != nil {
		return err
	}

	fmt.Printf("Successfully loaded coverage data for %d task iterations.\n", len(ac.data))
	return nil
}

// runDataTasks executes all the pending coverage tasks to populate the iteration data
func (ac *analyzeConfig) runDataTasks() error {
	fmt.Printf("Processing coverage data for %d task iterations using %d parallel jobs...\n", len(ac.data), ac.numProcesses)

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	var errs []error
	var errsMutex sync.Mutex

	dataChan := make(chan *IterationData, len(ac.data))
	for i := range ac.data {
		dataChan <- &ac.data[i]
	}
	close(dataChan)

	var wg sync.WaitGroup
	for i := 0; i < ac.numProcesses; i++ {
		wg.Go(func() {
			for {
				select {
				case <-ctx.Done():
					return
				default:
				}

				select {
				case <-ctx.Done():
					return
				case d, ok := <-dataChan:
					if !ok {
						return
					}
					if err := getOrRunCoverage(ac.taskConfig, d); err != nil {
						errsMutex.Lock()
						errs = append(errs, err)
						errsMutex.Unlock()
						cancel()
						return
					}
				}
			}
		})
	}

	wg.Wait()

	if len(errs) > 0 {
		return errors.Join(errs...)
	}
	return nil
}

// calculateMachineDataTasks calculates tasks for gathering coverage data for all the iterations run by a machine.
func (ac *analyzeConfig) calculateMachineDataTasks(machine string) ([]IterationData, error) {
	machineDir := filepath.Join(ac.resultsDir, machine)
	var machineData []IterationData

	for _, fuzzer := range ac.settings.Fuzzers {
		fuzzerData, err := ac.calculateFuzzerDataTasks(machine, machineDir, fuzzer)
		if err != nil {
			return nil, err
		}
		machineData = append(machineData, fuzzerData...)
	}
	return machineData, nil
}

// calculateFuzzerDataTasks calculates tasks for gathering coverage data for a fuzzer across its supported corpora.
func (ac *analyzeConfig) calculateFuzzerDataTasks(machine, machineDir, fuzzer string) ([]IterationData, error) {
	var corpora []CorpusDef
	cfg, ok := fuzzerConfigs[fuzzer]
	if !ok {
		return nil, fmt.Errorf("unknown fuzzer: %s", fuzzer)
	}

	switch cfg.mode {
	case FuzzModeWgsl:
		corpora = ac.settings.WgslCorpora
	case FuzzModeIr:
		corpora = ac.settings.IrCorpora
	default:
		return nil, fmt.Errorf("unknown fuzz mode %d", cfg.mode)
	}

	var fuzzerData []IterationData
	for _, corpus := range corpora {
		corpusData, err := ac.calculateCorpusDataTasks(machine, machineDir, fuzzer, corpus)
		if err != nil {
			return nil, err
		}
		fuzzerData = append(fuzzerData, corpusData...)
	}
	return fuzzerData, nil
}

// calculateCorpusDataTasks calculates tasks for gathering coverage data for a specific fuzzer and corpus across all configured durations and iterations.
func (ac *analyzeConfig) calculateCorpusDataTasks(machine, machineDir, fuzzer string, corpus CorpusDef) ([]IterationData, error) {
	var corpusData []IterationData
	for _, dDef := range ac.settings.Durations {
		limitType := "seconds"
		limitValue := 0
		if dDef.Seconds != nil {
			limitType = "seconds"
			limitValue = *dDef.Seconds
		} else if dDef.Runs != nil {
			limitType = "runs"
			limitValue = *dDef.Runs
		}

		iterations := ac.settings.DefaultIterations
		if dDef.Iterations != nil {
			iterations = *dDef.Iterations
		}

		limitStr := fmt.Sprintf("%s_%d", limitType, limitValue)

		for i := 1; i <= iterations; i++ {
			iterData, err := ac.calculateIterationCoverageTasks(machine, machineDir, fuzzer, corpus, limitType, limitValue, limitStr, i)
			if err != nil {
				return nil, err
			}
			if iterData != nil {
				corpusData = append(corpusData, *iterData)
			}
		}
	}
	return corpusData, nil
}

// calculateIterationCoverageTasks calculates a task for gathering coverage data for a single fuzzer iteration.
func (ac *analyzeConfig) calculateIterationCoverageTasks(machine, machineDir, fuzzer string, corpus CorpusDef, limitType string, limitValue int, limitStr string, iter int) (*IterationData, error) {
	iterDir := filepath.Join(machineDir, fuzzer, corpus.Name, limitStr, fmt.Sprintf("iter_%d", iter))
	statePath := filepath.Join(iterDir, "state.json")

	stateBytes, err := ac.osWrapper.ReadFile(statePath)
	if err != nil {
		return nil, err
	}

	var state IterState
	if err := json.Unmarshal(stateBytes, &state); err != nil {
		return nil, err
	}

	if state.Status != "completed" {
		return nil, fmt.Errorf("incomplete run for %s/%s iter %d\n", fuzzer, corpus.Name, iter)
	}

	if state.PerfScore <= 0 {
		return nil, fmt.Errorf("invalid perf score (%f) for %s/%s iter %d\n", state.PerfScore, fuzzer, corpus.Name, iter)
	}

	// Normalization: CPU Seconds = (ActualSeconds * PerfScore) / 1000
	normalizedSecs := (state.ActualSeconds * state.PerfScore) / 1000.0

	return &IterationData{
		Machine:        machine,
		Fuzzer:         fuzzer,
		Corpus:         corpus.Name,
		LimitType:      limitType,
		LimitValue:     limitValue,
		Iteration:      iter,
		PerfScore:      state.PerfScore,
		ActualSeconds:  state.ActualSeconds,
		ActualRuns:     state.ActualRuns,
		NormalizedSecs: normalizedSecs,
	}, nil
}

// printRawCSV builds and writes a CSV file containing the raw metrics of each fuzzer iteration.
func (ac *analyzeConfig) printRawCSV() error {
	var csvBuilder strings.Builder
	csvBuilder.WriteString("Machine,Fuzzer,Corpus,LimitType,LimitValue,Iteration,PerfScore,ActualSeconds,ActualRuns,NormalizedCPUSeconds," +
		"TintCore_LinesFound,TintCore_LinesHit,TintCore_CoveragePercent," +
		"Mesa_LinesFound,Mesa_LinesHit,Mesa_CoveragePercent," +
		"DirectX_LinesFound,DirectX_LinesHit,DirectX_CoveragePercent\n")

	for _, d := range ac.data {
		csvBuilder.WriteString(fmt.Sprintf("%s,%s,%s,%s,%d,%d,%.4f,%.2f,%d,%.4f,%d,%d,%.2f,%d,%d,%.2f,%d,%d,%.2f\n",
			d.Machine,
			d.Fuzzer,
			d.Corpus,
			d.LimitType,
			d.LimitValue,
			d.Iteration,
			d.PerfScore,
			d.ActualSeconds,
			d.ActualRuns,
			d.NormalizedSecs,
			d.Coverage.TintCore.LinesFound,
			d.Coverage.TintCore.LinesHit,
			d.Coverage.TintCore.Percentage,
			d.Coverage.Mesa.LinesFound,
			d.Coverage.Mesa.LinesHit,
			d.Coverage.Mesa.Percentage,
			d.Coverage.DirectX.LinesFound,
			d.Coverage.DirectX.LinesHit,
			d.Coverage.DirectX.Percentage,
		))
	}

	csvFile := filepath.Join(ac.analyzePath, kAnalyzeCsvFile)
	if err := ac.osWrapper.WriteFile(csvFile, []byte(csvBuilder.String()), 0644); err != nil {
		return fmt.Errorf("failed to write raw CSV file: %w", err)
	}

	fmt.Printf("Raw iteration metrics exported successfully to: %s\n", csvFile)
	return nil
}

// calculateStats computes statistical summaries for coverage percentages,
// normalized CPU seconds, and coverage rates across fuzzer runs.
func calculateStats(data []IterationData) []SummaryPoint {
	type accumulatorKey struct {
		Fuzzer     string
		Corpus     string
		LimitType  string
		LimitValue int
		Component  string
	}

	type accumulatorVal struct {
		NormalizedSecs  []float64
		CoveragePercent []float64
	}

	accumulations := make(map[accumulatorKey]*accumulatorVal)

	for _, d := range data {
		components := []struct {
			Name  string
			Stats CoverageStats
		}{
			{"Tint Core", d.Coverage.TintCore},
			{"Mesa", d.Coverage.Mesa},
			{"DirectX", d.Coverage.DirectX},
		}

		for _, c := range components {
			if c.Stats.LinesFound == 0 {
				continue // skip reporting empty components (e.g. Mesa if not a Mesa fuzzer)
			}

			key := accumulatorKey{
				Fuzzer:     d.Fuzzer,
				Corpus:     d.Corpus,
				LimitType:  d.LimitType,
				LimitValue: d.LimitValue,
				Component:  c.Name,
			}
			val, ok := accumulations[key]
			if !ok {
				val = &accumulatorVal{}
				accumulations[key] = val
			}

			val.NormalizedSecs = append(val.NormalizedSecs, d.NormalizedSecs)
			val.CoveragePercent = append(val.CoveragePercent, c.Stats.Percentage)
		}
	}

	// Sort keys for deterministic output
	var keys []accumulatorKey
	for k := range accumulations {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool {
		if keys[i].Fuzzer != keys[j].Fuzzer {
			return keys[i].Fuzzer < keys[j].Fuzzer
		}
		if keys[i].Corpus != keys[j].Corpus {
			return keys[i].Corpus < keys[j].Corpus
		}
		if keys[i].Component != keys[j].Component {
			return keys[i].Component < keys[j].Component
		}
		if keys[i].LimitType != keys[j].LimitType {
			return keys[i].LimitType < keys[j].LimitType
		}
		return keys[i].LimitValue < keys[j].LimitValue
	})

	var summaries []SummaryPoint
	for _, k := range keys {
		val := accumulations[k]
		coverageAvg, coverageStd := computeAvgAndStdDev(val.CoveragePercent)
		normalizedSecsAvg, normalizedSecsStd := computeAvgAndStdDev(val.NormalizedSecs)
		N := len(val.CoveragePercent)

		// deltaX is the Standard Error of Mean for a value X.
		// δX = σC / sqrt(N), where σFoo is the standard deviation of Foo, and N is the number of samples of Foo.
		deltaCoverage := 0.0
		if N > 0 {
			deltaCoverage = coverageStd / math.Sqrt(float64(N))
		}

		deltaNormalizedSecs := 0.0
		if N > 0 {
			deltaNormalizedSecs = normalizedSecsStd / math.Sqrt(float64(N))
		}

		coverageRate := 0.0
		if normalizedSecsAvg > 0 {
			coverageRate = coverageAvg / normalizedSecsAvg
		}

		// Quadrature error propagation for R, rate of a value X
		// R = X / Time
		// δR = R * sqrt((δX / X)^2 + (δT / T)^2)
		deltaCoverageRate := 0.0
		if coverageRate > 0 && coverageAvg > 0 && normalizedSecsAvg > 0 {
			relativeDeltaCoverage := deltaCoverage / coverageAvg
			relativeDeltaNormalizedSecs := deltaNormalizedSecs / normalizedSecsAvg
			deltaCoverageRate = coverageRate * math.Sqrt(relativeDeltaCoverage*relativeDeltaCoverage+relativeDeltaNormalizedSecs*relativeDeltaNormalizedSecs)
		}

		summaries = append(summaries, SummaryPoint{
			Fuzzer:       k.Fuzzer,
			Corpus:       k.Corpus,
			Component:    k.Component,
			LimitType:    k.LimitType,
			LimitValue:   k.LimitValue,
			NormSecsAvg:  normalizedSecsAvg,
			NormSecsSem:  deltaNormalizedSecs,
			CovAvg:       coverageAvg,
			CovSem:       deltaCoverage,
			CovRate:      coverageRate,
			CovRateError: deltaCoverageRate,
			N:            N,
		})
	}

	return summaries
}

// printStatsCSV builds and writes a CSV file containing the calculated statistics.
func (ac *analyzeConfig) printStatsCSV(stats []SummaryPoint) error {
	var csvBuilder strings.Builder
	csvBuilder.WriteString("Fuzzer,Corpus,Component,Samples,LimitType,LimitValue,NormalizedCPUSecondsAvg,NormalizedCPUSecondsSEM,CoveragePercentAvg,CoveragePercentSEM,CoverageRateAvg,CoverageRateSEM\n")

	for _, pt := range stats {
		csvBuilder.WriteString(fmt.Sprintf("%s,%s,%s,%d,%s,%d,%.4f,%.4f,%.2f,%.2f,%.6f,%.6f\n",
			pt.Fuzzer,
			pt.Corpus,
			pt.Component,
			pt.N,
			pt.LimitType,
			pt.LimitValue,
			pt.NormSecsAvg,
			pt.NormSecsSem,
			pt.CovAvg,
			pt.CovSem,
			pt.CovRate,
			pt.CovRateError,
		))
	}

	csvFile := filepath.Join(ac.analyzePath, kAnalyzeStatsCsvFile)
	if err := ac.osWrapper.WriteFile(csvFile, []byte(csvBuilder.String()), 0644); err != nil {
		return fmt.Errorf("failed to write calculated statistics CSV file: %w", err)
	}

	fmt.Printf("Calculated statistics exported successfully to: %s\n", csvFile)
	return nil
}

// printReport builds and writes a Markdown report summarizing fuzzer performance and coverage.
func (ac *analyzeConfig) printReport(stats []SummaryPoint) error {
	var reportBuilder strings.Builder
	reportBuilder.WriteString(fmt.Sprintf("# Experiment Performance and Coverage Report: %s\n\n", ac.settings.Name))
	reportBuilder.WriteString(fmt.Sprintf("- **Git Hash**: `%s`\n", ac.settings.Hash))
	if ac.machineName != "" {
		reportBuilder.WriteString(fmt.Sprintf("- **Target Machine**: `%s`\n", ac.machineName))
	} else {
		reportBuilder.WriteString("- **Machines Merged**:\n")
		for _, m := range ac.machines {
			reportBuilder.WriteString(fmt.Sprintf("  - `%s`\n", m))
		}
	}

	reportBuilder.WriteString("\n## Detailed Coverage Summaries\n\n")

	headers := []string{"Samples (N)", "Target Limit", "Normalized CPU Seconds (Avg ± SEM)", "Coverage % (Avg ± SEM)", "Coverage Rate (%/sec) (Avg ± SEM)"}
	var currentGroup string
	var rows [][]string

	flushTable := func() {
		if len(rows) > 0 {
			reportBuilder.WriteString(fmt.Sprintf("### %s\n\n", currentGroup))
			reportBuilder.WriteString(formatMarkdownTable(headers, rows))
			reportBuilder.WriteString("\n")
			rows = nil
		}
	}

	for _, pt := range stats {
		group := fmt.Sprintf("%s - %s (%s)", pt.Fuzzer, pt.Corpus, pt.Component)
		if group != currentGroup {
			flushTable()
			currentGroup = group
		}

		limitStr := fmt.Sprintf("%d %s", pt.LimitValue, pt.LimitType)
		normSecsStr := fmt.Sprintf("%.2f ± %.2f", pt.NormSecsAvg, pt.NormSecsSem)
		covStr := fmt.Sprintf("%.2f%% ± %.2f%%", pt.CovAvg, pt.CovSem)
		covRateStr := fmt.Sprintf("%.6f ± %.6f", pt.CovRate, pt.CovRateError)
		nStr := fmt.Sprintf("%d", pt.N)

		rows = append(rows, []string{
			nStr,
			limitStr,
			normSecsStr,
			covStr,
			covRateStr,
		})
	}
	flushTable()

	// Save Report File
	reportFile := filepath.Join(ac.analyzePath, kAnalyzeReportFile)
	if err := ac.osWrapper.WriteFile(reportFile, []byte(reportBuilder.String()), 0644); err != nil {
		return fmt.Errorf("failed to write report file: %w", err)
	}

	fmt.Printf("\nReport generated successfully at: %s\n", reportFile)
	fmt.Println("\n--- SUMMARY REPORT ---")
	fmt.Println(reportBuilder.String()[:min(1500, len(reportBuilder.String()))] + "...\n[Report truncated in output]")
	fmt.Println("----------------------")

	return nil
}

// getOrRunCoverage retrieves calculated coverage data for an iteration or executes
// the coverage collection if the data isn't present.
func getOrRunCoverage(t *taskConfig, d *IterationData) error {
	limitStr := fmt.Sprintf("%s_%d", d.LimitType, d.LimitValue)
	iterDir := filepath.Join(t.analyzePath, kAnalyzeResultsDir, d.Machine, d.Fuzzer, d.Corpus, limitStr, fmt.Sprintf("iter_%d", d.Iteration))

	coverageDir, err := filepath.Abs(filepath.Join(t.analyzePath, "coverage", d.Machine, d.Fuzzer, d.Corpus, limitStr, fmt.Sprintf("iter_%d", d.Iteration)))
	if err != nil {
		return err
	}
	coverageJsonPath := filepath.Join(coverageDir, "coverage.json")

	// If coverage already exists, load and return
	if fileutils.IsFile(coverageJsonPath, t.osWrapper) {
		covBytes, err := t.osWrapper.ReadFile(coverageJsonPath)
		if err == nil {
			var coverage IterationCoverage
			if err := json.Unmarshal(covBytes, &coverage); err == nil {
				d.Coverage = coverage
				return nil
			}
		}
	}

	// Ensure coverage dir exists
	if err := t.osWrapper.MkdirAll(coverageDir, 0755); err != nil {
		return fmt.Errorf("failed to create coverage dir: %w", err)
	}

	binDir, err := filepath.Abs(filepath.Join(t.analyzePath, "bin"))
	if err != nil {
		return err
	}

	// Find .profraw files inside the original iteration directory
	profrawFiles, err := findProfrawFiles(iterDir)
	if err != nil {
		return fmt.Errorf("failed to scan for .profraw files inside %s: %w", iterDir, err)
	}

	if len(profrawFiles) == 0 {
		return fmt.Errorf("no .profraw file found inside iteration directory '%s' (make sure the experiment ran with coverage enabled)", iterDir)
	}

	profdataPath, err := filepath.Abs(filepath.Join(iterDir, "coverage.profdata"))
	if err != nil {
		return err
	}

	if err := mergeProfrawFiles(t, binDir, profrawFiles, profdataPath); err != nil {
		return err
	}

	fmt.Printf("Generating coverage for %s/%s/%s (%s) iter %d...\n", d.Machine, d.Fuzzer, d.Corpus, limitStr, d.Iteration)
	if err := generateLcovReport(t, d.Fuzzer, binDir, coverageDir, profdataPath); err != nil {
		return err
	}

	// Parse generated LCOV file
	lcovFile, err := findLcovFile(coverageDir)
	if err != nil {
		return fmt.Errorf("failed to find generated LCOV file: %w", err)
	}

	lcovContent, err := t.osWrapper.ReadFile(lcovFile)
	if err != nil {
		return fmt.Errorf("failed to read generated LCOV file: %w", err)
	}

	d.Coverage = parseLcov(string(lcovContent))

	// Cache result
	coverageBytes, _ := json.MarshalIndent(d.Coverage, "", "  ")
	_ = t.osWrapper.WriteFile(coverageJsonPath, coverageBytes, 0644)

	return nil
}

// findProfrawFiles recursively searches for and returns a list of all .profraw files within the specified directory.
func findProfrawFiles(dir string) ([]string, error) {
	var files []string
	err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() && filepath.Ext(path) == ".profraw" {
			files = append(files, path)
		}
		return nil
	})
	return files, err
}

// mergeProfrawFiles combines multiple .profraw files into a single sparse .profdata file using llvm-profdata.
func mergeProfrawFiles(t *taskConfig, binDir string, inputs []string, output string) error {
	llvmProfDataPath := filepath.Join(binDir, "llvm-profdata"+fileutils.ExeExt)
	if !fileutils.IsExe(llvmProfDataPath, t.osWrapper) {
		return fmt.Errorf("hermetic LLVM tool 'llvm-profdata' not found in experiment bin directory: %s", llvmProfDataPath)
	}

	mergeArgs := []string{"merge", "-o", output, "-sparse=true"}
	mergeArgs = append(mergeArgs, inputs...)
	if _, err := t.runCmd(llvmProfDataPath, mergeArgs...); err != nil {
		return fmt.Errorf("failed to merge .profraw files using %s: %w", llvmProfDataPath, err)
	}
	return nil
}

// generateLcovReport runs coverage.py on a pre-generated coverage.profdata file.
func generateLcovReport(t *taskConfig, fuzzer string, binDir string, output string, inputs string) error {
	scriptPath := filepath.Join(fileutils.DawnRoot(t.osWrapper), "tools", "code_coverage", "coverage.py")

	llvmCovPath := filepath.Join(binDir, "llvm-cov"+fileutils.ExeExt)
	if !fileutils.IsExe(llvmCovPath, t.osWrapper) {
		return fmt.Errorf("hermetic LLVM tool 'llvm-cov' not found in experiment bin directory: %s", llvmCovPath)
	}

	cmdArgs := []string{
		scriptPath,
		fuzzer,
		"--no-component-view",
		"-b", binDir,
		"-o", output,
		"--format", "lcov",
		"-p", inputs,
		"--coverage-tools-dir", binDir,
	}

	if _, err := t.runCmd("vpython3", cmdArgs...); err != nil {
		return fmt.Errorf("failed to execute coverage.py: %w", err)
	}
	return nil
}

// findLcovFile searches for the first .lcov file within the specified directory.
func findLcovFile(dir string) (string, error) {
	var foundPath string
	err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() && filepath.Ext(path) == ".lcov" {
			foundPath = path
			return filepath.SkipAll // Stop searching
		}
		return nil
	})
	if err != nil && !errors.Is(err, filepath.SkipAll) {
		return "", err
	}
	if foundPath == "" {
		return "", fmt.Errorf("no LCOV file found in %s", dir)
	}
	return foundPath, nil
}

// parseLcov parses the contents of an LCOV file and categorizes coverage metrics
// into Tint Core, Mesa, and DirectX components based on file paths.
func parseLcov(content string) IterationCoverage {
	lines := strings.Split(content, "\n")
	var currentFile string
	var inTintCore, inMesa, inDirectX bool

	metrics := map[string]*CoverageStats{
		"tint_core": {},
		"mesa":      {},
		"directx":   {},
	}

	for _, line := range lines {
		line = strings.TrimSpace(line)
		if after, ok := strings.CutPrefix(line, "SF:"); ok {
			currentFile = after
			currentFile = filepath.ToSlash(currentFile)

			inTintCore = strings.Contains(currentFile, "src/tint/") || strings.Contains(currentFile, "src/utils/")
			inMesa = strings.Contains(currentFile, "third_party/mesa/")
			inDirectX = strings.Contains(currentFile, "third_party/directx")
		} else if after, ok := strings.CutPrefix(line, "LF:"); ok {
			val, _ := strconv.Atoi(after)
			if inTintCore {
				metrics["tint_core"].LinesFound += val
			}
			if inMesa {
				metrics["mesa"].LinesFound += val
			}
			if inDirectX {
				metrics["directx"].LinesFound += val
			}
		} else if after, ok := strings.CutPrefix(line, "LH:"); ok {
			val, _ := strconv.Atoi(after)
			if inTintCore {
				metrics["tint_core"].LinesHit += val
			}
			if inMesa {
				metrics["mesa"].LinesHit += val
			}
			if inDirectX {
				metrics["directx"].LinesHit += val
			}
		}
	}

	for _, stats := range metrics {
		if stats.LinesFound > 0 {
			stats.Percentage = (float64(stats.LinesHit) / float64(stats.LinesFound)) * 100.0
		}
	}

	return IterationCoverage{
		TintCore: *metrics["tint_core"],
		Mesa:     *metrics["mesa"],
		DirectX:  *metrics["directx"],
	}
}

// computeAvgAndStdDev calculates the arithmetic mean and sample standard deviation for a slice of float64.
func computeAvgAndStdDev(vals []float64) (avg float64, sd float64) {
	if len(vals) == 0 {
		return 0, 0
	}
	sum := 0.0
	for _, v := range vals {
		sum += v
	}
	avg = sum / float64(len(vals))

	if len(vals) < 2 {
		return avg, 0
	}

	sqSum := 0.0
	for _, v := range vals {
		diff := v - avg
		sqSum += diff * diff
	}
	variance := sqSum / float64(len(vals)-1)
	sd = math.Sqrt(variance)
	return avg, sd
}

func formatMarkdownTable(headers []string, rows [][]string) string {
	numCols := len(headers)
	colWidths := make([]int, numCols)

	// Calculate column widths from headers
	for i, h := range headers {
		colWidths[i] = utf8.RuneCountInString(h)
	}

	// Calculate column widths from rows
	for _, row := range rows {
		for i, val := range row {
			valLen := utf8.RuneCountInString(val)
			if i < numCols && valLen > colWidths[i] {
				colWidths[i] = valLen
			}
		}
	}

	var sb strings.Builder

	// Write header row
	sb.WriteString("|")
	for i, h := range headers {
		padding := colWidths[i] - utf8.RuneCountInString(h)
		sb.WriteString(" " + h + strings.Repeat(" ", padding) + " |")
	}
	sb.WriteString("\n")

	// Write separator row
	sb.WriteString("|")
	for i := range numCols {
		sb.WriteString(" " + strings.Repeat("-", colWidths[i]) + " |")
	}
	sb.WriteString("\n")

	// Write data rows
	for _, row := range rows {
		sb.WriteString("|")
		for i, val := range row {
			padding := 0
			if i < numCols {
				padding = colWidths[i] - utf8.RuneCountInString(val)
			}
			sb.WriteString(" " + val + strings.Repeat(" ", padding) + " |")
		}
		sb.WriteString("\n")
	}

	return sb.String()
}
