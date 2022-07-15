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

package main

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"log"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
	"regexp"
	"sort"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/bench"
	"dawn.googlesource.com/dawn/tools/src/git"
	"github.com/andygrunwald/go-gerrit"
	"github.com/shirou/gopsutil/cpu"
)

// main entry point
func main() {
	var cfgPath string
	flag.StringVar(&cfgPath, "c", "~/.config/perfmon/config.json", "the config file")
	flag.Parse()

	if err := run(cfgPath); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

// run starts the perfmon tool with the given config path
func run(cfgPath string) error {
	cfgPath, err := expandHomeDir(cfgPath)
	if err != nil {
		return err
	}

	if err := findTools(); err != nil {
		return err
	}

	g, err := git.New(tools.git)
	if err != nil {
		return err
	}

	cfg, err := loadConfig(cfgPath)
	if err != nil {
		return err
	}

	dawnDir, resultsDir, err := makeWorkingDirs(cfg)
	if err != nil {
		return err
	}
	dawnRepo, err := createOrOpenGitRepo(g, dawnDir, cfg.Dawn)
	if err != nil {
		return err
	}
	resultsRepo, err := createOrOpenGitRepo(g, resultsDir, cfg.Results)
	if err != nil {
		return err
	}
	gerritClient, err := gerrit.NewClient(cfg.Gerrit.URL, nil)
	if err != nil {
		return err
	}
	gerritClient.Authentication.SetBasicAuth(cfg.Gerrit.Username, cfg.Gerrit.Password)

	sysInfo, err := cpu.Info()
	if err != nil {
		return fmt.Errorf("failed to obtain system info:\n  %v", err)
	}

	// Some machines report slightly different CPU clock speeds each reboot
	// To work around this, quantize the reported speed to the nearest 100MHz
	for i, s := range sysInfo {
		sysInfo[i].Mhz = math.Round(s.Mhz/100) * 100
	}

	e := env{
		cfg:         cfg,
		git:         g,
		system:      sysInfo,
		systemID:    hash(sysInfo)[:8],
		dawnDir:     dawnDir,
		buildDir:    filepath.Join(dawnDir, "out"),
		resultsDir:  resultsDir,
		dawnRepo:    dawnRepo,
		resultsRepo: resultsRepo,
		gerrit:      gerritClient,

		benchmarkCache: map[git.Hash]*bench.Run{},
	}

	for {
		didSomething, err := e.doSomeWork()
		if err != nil {
			log.Printf("ERROR: %v", err)
			log.Printf("Pausing...")
			time.Sleep(time.Minute * 10)
			continue
		}
		if !didSomething {
			log.Println("nothing to do. Sleeping...")
			time.Sleep(time.Minute * 5)
		}
	}
}

// Config holds the root configuration options for the perfmon tool
type Config struct {
	WorkingDir           string
	RootChange           git.Hash
	Dawn                 GitConfig
	Results              GitConfig
	Gerrit               GerritConfig
	Timeouts             TimeoutsConfig
	ExternalAccounts     []string
	BenchmarkRepetitions int
	BenchmarkMaxTemp     float32 // celsius
	CPUTempSensorName    string  // Name of the sensor to use for CPU temp
}

// GitConfig holds the configuration options for accessing a git repo
type GitConfig struct {
	URL         string
	Branch      string
	Credentials git.Credentials
}

// GerritConfig holds the configuration options for accessing gerrit
type GerritConfig struct {
	URL      string
	Username string
	Email    string
	Password string
}

// TimeoutsConfig holds the configuration options for timeouts
type TimeoutsConfig struct {
	Sync      time.Duration
	Build     time.Duration
	Benchmark time.Duration
}

// HistoricResults contains the full set of historic benchmark results for a
// given system
type HistoricResults struct {
	System  []cpu.InfoStat
	Commits []CommitResults
}

// CommitResults holds the results of a single dawn commit
type CommitResults struct {
	Commit            string
	CommitTime        time.Time
	CommitDescription string
	Benchmarks        []Benchmark
}

// Benchmark holds the benchmark results for a single test
type Benchmark struct {
	Name    string
	Time    float64
	Repeats int `json:",omitempty"`
}

// AuthConfig holds the authentication options for accessing a git repo
type AuthConfig struct {
	Username string
	Password string
}

// setDefaults assigns default values to unassigned fields of cfg
func (cfg *Config) setDefaults() {
	if cfg.RootChange.IsZero() {
		cfg.RootChange, _ = git.ParseHash("e72e42d9e0c851311512ca6da4d7b59f0bcc60d9")
	}
	cfg.Dawn.setDefaults()
	cfg.Results.setDefaults()
	cfg.Timeouts.setDefaults()
	if cfg.BenchmarkRepetitions < 1 {
		cfg.BenchmarkRepetitions = 1
	}
	if cfg.BenchmarkMaxTemp == 0 {
		cfg.BenchmarkMaxTemp = 50
	}
}

// setDefaults assigns default values to unassigned fields of cfg
func (cfg *GitConfig) setDefaults() {
	if cfg.Branch == "" {
		cfg.Branch = "main"
	}
}

// setDefaults assigns default values to unassigned fields of cfg
func (cfg *TimeoutsConfig) setDefaults() {
	if cfg.Sync == 0 {
		cfg.Sync = time.Minute * 10
	}
	if cfg.Build == 0 {
		cfg.Build = time.Minute * 10
	}
	if cfg.Benchmark == 0 {
		cfg.Benchmark = time.Minute * 30
	}
}

// findCommitResults looks for a CommitResult with the given commit id,
// returning a pointer to the CommitResult if found, otherwise nil
func (h *HistoricResults) findCommitResults(commit string) *CommitResults {
	for i, c := range h.Commits {
		if c.Commit == commit {
			return &h.Commits[i]
		}
	}
	return nil
}

// sorts all the benchmarks by commit date
func (h *HistoricResults) sort() {
	sort.Slice(h.Commits, func(i, j int) bool {
		if h.Commits[i].CommitTime.Before(h.Commits[j].CommitTime) {
			return true
		}
		if h.Commits[j].CommitTime.Before(h.Commits[i].CommitTime) {
			return false
		}
		return h.Commits[i].CommitDescription < h.Commits[j].CommitDescription
	})
}

// findBenchmark looks for a Benchmark with the given commit id,
// returning a pointer to the Benchmark if found, otherwise nil
func (r *CommitResults) findBenchmark(name string) *Benchmark {
	for i, b := range r.Benchmarks {
		if b.Name == name {
			return &r.Benchmarks[i]
		}
	}
	return nil
}

// sorts all the benchmarks by name
func (r *CommitResults) sort() {
	sort.Slice(r.Benchmarks, func(i, j int) bool {
		return r.Benchmarks[i].Name < r.Benchmarks[j].Name
	})
}

// env holds the perfmon main environment state
type env struct {
	cfg         Config
	git         *git.Git
	system      []cpu.InfoStat
	systemID    string
	dawnDir     string
	buildDir    string
	resultsDir  string
	dawnRepo    *git.Repository
	resultsRepo *git.Repository
	gerrit      *gerrit.Client

	benchmarkCache map[git.Hash]*bench.Run
}

// doSomeWork scans gerrit for changes up for review and submitted changes to
// benchmark. If something was found to do, then returns true.
func (e env) doSomeWork() (bool, error) {
	{
		log.Println("scanning for review changes to benchmark...")
		change, err := e.findGerritChangeToBenchmark()
		if err != nil {
			return true, err
		}
		if change != nil {
			if err := e.benchmarkGerritChange(*change); err != nil {
				return true, err
			}
			return true, nil
		}
	}

	{
		log.Println("scanning for submitted changes to benchmark...")
		changesToBenchmark, err := e.changesToBenchmark()
		if err != nil {
			return true, err
		}

		if len(changesToBenchmark) > 0 {
			log.Printf("%v submitted changes to benchmark...", len(changesToBenchmark))

			start := time.Now()
			for i, c := range changesToBenchmark {
				if time.Since(start) > time.Minute*15 {
					// It's been a while since we scanned for review changes.
					// Take a break from benchmarking submitted changes so we
					// can scan for review changes to benchmark.
					log.Printf("benchmarked %v changes", i)
					return true, nil
				}
				benchRes, err := e.benchmarkTintChange(c)
				if err != nil {
					log.Printf("benchmarking failed: %v", err)
					benchRes = &bench.Run{}
				}
				commitRes, err := e.benchmarksToCommitResults(c, *benchRes)
				if err != nil {
					return true, err
				}
				log.Printf("pushing results...")
				if err := e.pushUpdatedResults(*commitRes); err != nil {
					return true, err
				}
			}
			return true, nil
		}
	}

	{
		log.Println("scanning for benchmarks to refine...")
		changeToBenchmark, err := e.changeToRefineBenchmarks()
		if err != nil {
			return true, err
		}

		if changeToBenchmark != nil {
			log.Printf("re-benchmarking change '%v'", *changeToBenchmark)
			benchRes, err := e.benchmarkTintChange(*changeToBenchmark)
			if err != nil {
				log.Printf("benchmarking failed: %v", err)
				benchRes = &bench.Run{}
			}
			commitRes, err := e.benchmarksToCommitResults(*changeToBenchmark, *benchRes)
			if err != nil {
				return true, err
			}
			log.Printf("pushing results...")
			if err := e.pushUpdatedResults(*commitRes); err != nil {
				return true, err
			}
			return true, nil
		}
	}
	return false, nil
}

// changesToBenchmark fetches the list of changes that do not currently have
// benchmark results, which should be benchmarked.
func (e env) changesToBenchmark() ([]git.Hash, error) {
	log.Println("syncing dawn repo...")
	latest, err := e.dawnRepo.Fetch(e.cfg.Dawn.Branch, &git.FetchOptions{
		Credentials: e.cfg.Dawn.Credentials,
	})
	if err != nil {
		return nil, err
	}
	allChanges, err := e.dawnRepo.Log(&git.LogOptions{
		From: e.cfg.RootChange.String(),
		To:   latest.String(),
	})
	if err != nil {
		return nil, fmt.Errorf("failed to obtain dawn log:\n  %w", err)
	}
	changesWithBenchmarks, err := e.changesWithBenchmarks()
	if err != nil {
		return nil, fmt.Errorf("failed to gather changes with existing benchmarks:\n  %w", err)
	}
	changesToBenchmark := make([]git.Hash, 0, len(allChanges))
	for _, c := range allChanges {
		if _, exists := changesWithBenchmarks[c.Hash]; !exists {
			changesToBenchmark = append(changesToBenchmark, c.Hash)
		}
	}
	// Reverse the order of changesToBenchmark, so that the oldest comes first.
	for i := len(changesToBenchmark)/2 - 1; i >= 0; i-- {
		j := len(changesToBenchmark) - 1 - i
		changesToBenchmark[i], changesToBenchmark[j] = changesToBenchmark[j], changesToBenchmark[i]
	}

	return changesToBenchmark, nil
}

// changeToRefineBenchmarks scans for the most suitable historic commit to
// re-benchmark and refine the results. Returns nil if there are no suitable
// changes.
func (e env) changeToRefineBenchmarks() (*git.Hash, error) {
	log.Println("syncing results repo...")
	if err := fetchAndCheckoutLatest(e.resultsRepo, e.cfg.Results); err != nil {
		return nil, err
	}

	_, absPath, err := e.resultsFilePaths()
	if err != nil {
		return nil, err
	}

	results, err := e.loadHistoricResults(absPath)
	if err != nil {
		log.Println(fmt.Errorf("WARNING: failed to open result file '%v':\n  %w", absPath, err))
		return nil, nil
	}

	if len(results.Commits) == 0 {
		return nil, nil
	}

	type hashDelta struct {
		hash  git.Hash
		delta float64
	}
	hashDeltas := make([]hashDelta, 0, len(results.Commits))
	for i, c := range results.Commits {
		hash, err := git.ParseHash(c.Commit)
		if err != nil {
			return nil, err
		}

		prev := results.Commits[max(0, i-1)]
		next := results.Commits[min(len(results.Commits)-1, i+1)]
		delta, count := 0.0, 0
		for _, b := range c.Benchmarks {
			if b.Time == 0 {
				continue
			}
			p, n := b.Time, b.Time
			if pb := prev.findBenchmark(b.Name); pb != nil {
				p = pb.Time
			}
			if nb := next.findBenchmark(b.Name); nb != nil {
				n = nb.Time
			}
			avr := (p + n) / 2
			confidence := math.Pow(2, float64(b.Repeats))
			delta += math.Abs(avr-b.Time) / (b.Time * confidence)
			count++
		}
		if count > 0 {
			delta = delta / float64(count)
			hashDeltas = append(hashDeltas, hashDelta{hash, delta})
		}
	}

	sort.Slice(hashDeltas, func(i, j int) bool { return hashDeltas[i].delta > hashDeltas[j].delta })

	return &hashDeltas[0].hash, nil
}

// benchmarkTintChangeIfNotCached first checks the results cache for existing
// benchmark values for the given change, returning those cached values if hit.
// If the cache does not contain results for the change, then
// e.benchmarkTintChange() is called.
func (e env) benchmarkTintChangeIfNotCached(hash git.Hash) (*bench.Run, error) {
	if cached, ok := e.benchmarkCache[hash]; ok {
		log.Printf("reusing cached benchmark results of '%v'...", hash)
		return cached, nil
	}
	return e.benchmarkTintChange(hash)
}

// benchmarkTintChange checks out the given commit, fetches the dawn third party
// dependencies, builds tint, then runs the benchmarks, returning the results.
func (e env) benchmarkTintChange(hash git.Hash) (*bench.Run, error) {
	log.Printf("checking out dawn at '%v'...", hash)
	if err := checkout(hash, e.dawnRepo); err != nil {
		return nil, err
	}
	log.Println("fetching dawn dependencies...")
	if err := e.fetchDawnDeps(); err != nil {
		return nil, err
	}
	log.Println("building tint...")
	if err := e.buildTint(); err != nil {
		return nil, err
	}
	if err := e.waitForTempsToSettle(); err != nil {
		return nil, err
	}
	log.Println("benchmarking tint...")
	run, err := e.repeatedlyBenchmarkTint()
	if err != nil {
		return nil, err
	}

	e.benchmarkCache[hash] = run
	return run, nil
}

// benchmarksToCommitResults converts the benchmarks in the provided bench.Run
// to a CommitResults.
func (e env) benchmarksToCommitResults(hash git.Hash, results bench.Run) (*CommitResults, error) {
	commits, err := e.dawnRepo.Log(&git.LogOptions{
		From: hash.String(),
	})
	if err != nil || len(commits) == 0 {
		return nil, fmt.Errorf("failed to get commit object '%v' of dawn repo:\n  %w", hash, err)
	}
	commit := commits[len(commits)-1]
	if commit.Hash != hash {
		panic(fmt.Errorf("git.Repository.Log({From: %v}) returned:\n%+v", hash, commits))
	}

	m := map[string]Benchmark{}
	for _, b := range results.Benchmarks {
		m[b.Name] = Benchmark{
			Name: b.Name,
			Time: float64(b.Duration) / float64(time.Second),
		}
	}

	out := &CommitResults{
		Commit:            commit.Hash.String(),
		CommitDescription: commit.Subject,
		CommitTime:        commit.Date,
		Benchmarks:        make([]Benchmark, 0, len(m)),
	}
	for _, b := range m {
		out.Benchmarks = append(out.Benchmarks, b)
	}
	out.sort()

	return out, nil
}

// changesWithBenchmarks returns a set of dawn changes that we already have
// benchmarks for.
func (e env) changesWithBenchmarks() (map[git.Hash]struct{}, error) {
	log.Println("syncing results repo...")
	if err := fetchAndCheckoutLatest(e.resultsRepo, e.cfg.Results); err != nil {
		return nil, err
	}

	_, absPath, err := e.resultsFilePaths()
	if err != nil {
		return nil, err
	}

	results, err := e.loadHistoricResults(absPath)
	if err != nil {
		log.Println(fmt.Errorf("WARNING: failed to open result file '%v':\n  %w", absPath, err))
		return nil, nil
	}

	m := make(map[git.Hash]struct{}, len(results.Commits))
	for _, c := range results.Commits {
		hash, err := git.ParseHash(c.Commit)
		if err != nil {
			return nil, err
		}
		m[hash] = struct{}{}
	}
	return m, nil
}

// pushUpdatedResults fetches and loads the latest benchmark results, adds or
// merges the new results 'res' to the file, and then pushes the new results to
// the server.
func (e env) pushUpdatedResults(res CommitResults) error {
	log.Println("syncing results repo...")
	if err := fetchAndCheckoutLatest(e.resultsRepo, e.cfg.Results); err != nil {
		return err
	}

	relPath, absPath, err := e.resultsFilePaths()
	if err != nil {
		return err
	}

	h, err := e.loadHistoricResults(absPath)
	if err != nil {
		log.Println(fmt.Errorf("failed to open result file '%v'. Creating new file\n  %w", absPath, err))
		h = &HistoricResults{System: e.system}
	}

	// Are there existing benchmark results for this commit?
	if existing := h.findCommitResults(res.Commit); existing != nil {
		// Yes: merge in the new results
		for _, b := range res.Benchmarks {
			if e := existing.findBenchmark(b.Name); e != nil {
				// Benchmark found to merge. Add a weighted contribution to the benchmark value.
				e.Time = (e.Time*float64(e.Repeats+1) + b.Time) / float64(e.Repeats+2)
				e.Repeats++
			} else {
				// New benchmark? Just append.
				existing.Benchmarks = append(existing.Benchmarks, b)
			}
		}
		existing.sort()
	} else {
		// New benchmark results for this commit. Just append.
		h.Commits = append(h.Commits, res)
	}

	// Sort the commits by timestamp
	h.sort()

	// Write the new results to the file
	f, err := os.Create(absPath)
	if err != nil {
		return fmt.Errorf("failed to create updated results file '%v':\n  %w", absPath, err)
	}
	defer f.Close()

	enc := json.NewEncoder(f)
	enc.SetIndent("", "  ")
	if err := enc.Encode(h); err != nil {
		return fmt.Errorf("failed to encode updated results file '%v':\n  %w", absPath, err)
	}

	// Stage the file
	if err := e.resultsRepo.Add(relPath, nil); err != nil {
		return fmt.Errorf("failed to stage updated results file '%v':\n  %w", relPath, err)
	}

	// Commit the change
	msg := fmt.Sprintf("Add benchmark results for '%v'", res.Commit[:6])
	hash, err := e.resultsRepo.Commit(msg, &git.CommitOptions{
		AuthorName:  "tint perfmon bot",
		AuthorEmail: "tint-perfmon-bot@gmail.com",
	})
	if err != nil {
		return fmt.Errorf("failed to commit updated results file '%v':\n  %w", absPath, err)
	}

	// Push the change
	log.Println("pushing updated results to results repo...")
	if err := e.resultsRepo.Push(hash.String(), e.cfg.Results.Branch, &git.PushOptions{
		Credentials: e.cfg.Results.Credentials,
	}); err != nil {
		return fmt.Errorf("failed to push updated results file '%v':\n  %w", absPath, err)
	}

	return nil
}

// resultsFilePaths returns the paths to the results.json file, holding the
// benchmarks for the given system.
func (e env) resultsFilePaths() (relPath string, absPath string, err error) {
	dir := filepath.Join(e.resultsDir, "results")
	if err = os.MkdirAll(dir, 0777); err != nil {
		err = fmt.Errorf("failed to create results directory '%v':\n  %w", dir, err)
		return
	}
	relPath = filepath.Join("results", e.systemID+".json")
	absPath = filepath.Join(dir, e.systemID+".json")
	return
}

// loadHistoricResults loads and returns the results.json file for the given
// system.
func (e env) loadHistoricResults(path string) (*HistoricResults, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("failed to open result file '%v':\n  %w", path, err)
	}
	defer file.Close()
	res := &HistoricResults{}
	if err := json.NewDecoder(file).Decode(res); err != nil {
		return nil, fmt.Errorf("failed to parse result file '%v':\n  %w", path, err)
	}

	if !reflect.DeepEqual(res.System, e.system) {
		log.Printf(`WARNING: results file '%v' has different system information!
File: %+v
System: %+v`, path, res.System, e.system)
	}

	return res, nil
}

// fetchDawnDeps fetches the third party dawn dependencies using gclient.
func (e env) fetchDawnDeps() error {
	gclientConfig := filepath.Join(e.dawnDir, ".gclient")
	if _, err := os.Stat(gclientConfig); errors.Is(err, os.ErrNotExist) {
		standalone := filepath.Join(e.dawnDir, "scripts", "standalone.gclient")
		if err := copyFile(gclientConfig, standalone); err != nil {
			return fmt.Errorf("failed to copy '%v' to '%v':\n  %w", standalone, gclientConfig, err)
		}
	}
	if _, err := call(tools.gclient, e.dawnDir, e.cfg.Timeouts.Sync,
		"sync",
		"--force",
	); err != nil {
		return fmt.Errorf("failed to fetch dawn dependencies:\n  %w", err)
	}
	return nil
}

// buildTint builds the tint benchmarks.
func (e env) buildTint() error {
	if err := os.MkdirAll(e.buildDir, 0777); err != nil {
		return fmt.Errorf("failed to create build directory at '%v':\n  %w", e.buildDir, err)
	}
	if _, err := call(tools.cmake, e.buildDir, e.cfg.Timeouts.Build,
		e.dawnDir,
		"-GNinja",
		"-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
		"-DCMAKE_BUILD_TYPE=Release",
		"-DCMAKE_BUILD_TESTS=0",
		"-DCMAKE_BUILD_SAMPLES=0",
		"-DTINT_BUILD_DOCS=0",
		"-DTINT_BUILD_SAMPLES=0",
		"-DTINT_BUILD_TESTS=0",
		"-DTINT_BUILD_SPV_READER=1",
		"-DTINT_BUILD_WGSL_READER=1",
		"-DTINT_BUILD_GLSL_WRITER=1",
		"-DTINT_BUILD_HLSL_WRITER=1",
		"-DTINT_BUILD_MSL_WRITER=1",
		"-DTINT_BUILD_SPV_WRITER=1",
		"-DTINT_BUILD_WGSL_WRITER=1",
		"-DTINT_BUILD_BENCHMARKS=1",
		"-DDAWN_BUILD_SAMPLES=0",
	); err != nil {
		return errFailedToBuild{fmt.Errorf("failed to generate dawn build config:\n  %w", err)}
	}
	if _, err := call(tools.ninja, e.buildDir, e.cfg.Timeouts.Build); err != nil {
		return errFailedToBuild{err}
	}
	return nil
}

// errFailedToBuild is the error returned by buildTint() if the build failed
type errFailedToBuild struct {
	// The reason
	reason error
}

func (e errFailedToBuild) Error() string {
	return fmt.Sprintf("failed to build: %v", e.reason)
}

// errFailedToBenchmark is the error returned by benchmarkTint() if the benchmark failed
type errFailedToBenchmark struct {
	// The reason
	reason error
}

func (e errFailedToBenchmark) Error() string {
	return fmt.Sprintf("failed to benchmark: %v", e.reason)
}

// benchmarkTint runs the tint benchmarks e.cfg.BenchmarkRepetitions times,
// returning the averaged results.
func (e env) repeatedlyBenchmarkTint() (*bench.Run, error) {
	type durationAndCount struct {
		duration time.Duration
		count    int
	}

	var ctx *bench.Context
	acc := map[string]durationAndCount{}
	for i := 0; i < e.cfg.BenchmarkRepetitions; i++ {
		if err := e.waitForTempsToSettle(); err != nil {
			return nil, err
		}
		log.Printf("benchmark pass %v/%v...", (i + 1), e.cfg.BenchmarkRepetitions)
		run, err := e.benchmarkTint()
		if err != nil {
			return nil, err
		}
		for _, b := range run.Benchmarks {
			v := acc[b.Name]
			v.duration += b.Duration
			v.count++
			acc[b.Name] = v
		}
		if ctx == nil {
			ctx = run.Context
		}
	}

	out := bench.Run{Context: ctx}
	for name, dc := range acc {
		out.Benchmarks = append(out.Benchmarks, bench.Benchmark{
			Name:     name,
			Duration: dc.duration / time.Duration(dc.count),
		})
	}

	return &out, nil
}

// benchmarkTint runs the tint benchmarks once, returning the results.
func (e env) benchmarkTint() (*bench.Run, error) {
	exe := filepath.Join(e.buildDir, "tint-benchmark")
	out, err := call(exe, e.buildDir, e.cfg.Timeouts.Benchmark,
		"--benchmark_format=json",
		"--benchmark_enable_random_interleaving=true",
	)
	if err != nil {
		return nil, errFailedToBenchmark{err}
	}

	results, err := bench.Parse(out)
	if err != nil {
		return nil, errFailedToBenchmark{err}
	}
	return &results, nil
}

// findGerritChangeToBenchmark queries gerrit for a change to benchmark.
func (e env) findGerritChangeToBenchmark() (*gerrit.ChangeInfo, error) {
	log.Println("querying gerrit for changes...")
	results, _, err := e.gerrit.Changes.QueryChanges(&gerrit.QueryChangeOptions{
		QueryOptions: gerrit.QueryOptions{
			Query: []string{"project:dawn status:open+-age:3d"},
			Limit: 100,
		},
		ChangeOptions: gerrit.ChangeOptions{
			AdditionalFields: []string{"CURRENT_REVISION", "CURRENT_COMMIT", "MESSAGES", "LABELS", "DETAILED_ACCOUNTS"},
		},
	})
	if err != nil {
		return nil, fmt.Errorf("failed to get list of changes:\n  %w", err)
	}

	type candidate struct {
		change   gerrit.ChangeInfo
		priority int
	}

	candidates := make([]candidate, 0, len(*results))

	for _, change := range *results {
		kokoroApproved := change.Labels["Kokoro"].Approved.AccountID != 0
		codeReviewScore := change.Labels["Code-Review"].Value
		codeReviewApproved := change.Labels["Code-Review"].Approved.AccountID != 0
		presubmitReady := change.Labels["Presubmit-Ready"].Approved.AccountID != 0
		verifiedScore := change.Labels["Verified"].Value

		current, ok := change.Revisions[change.CurrentRevision]
		if !ok {
			log.Printf("WARNING: couldn't find current revision for change '%s'", change.ChangeID)
		}

		canBenchmark := func() bool {
			// Is the change from a Googler, reviewed by a Googler or is from a allow-listed external developer?
			if !(strings.HasSuffix(current.Commit.Committer.Email, "@google.com") ||
				strings.HasSuffix(change.Labels["Code-Review"].Approved.Email, "@google.com") ||
				strings.HasSuffix(change.Labels["Code-Review"].Recommended.Email, "@google.com") ||
				strings.HasSuffix(change.Labels["Presubmit-Ready"].Approved.Email, "@google.com")) {
				permitted := false
				for _, email := range e.cfg.ExternalAccounts {
					if strings.EqualFold(current.Commit.Committer.Email, email) {
						permitted = true
						break
					}

				}
				if !permitted {
					return false
				}
			}

			// Don't benchmark if the change has negative scores.
			if codeReviewScore < 0 || verifiedScore < 0 {
				return false
			}

			// Has the latest patchset already been benchmarked?
			for _, msg := range change.Messages {
				if msg.RevisionNumber == current.Number &&
					msg.Author.Email == e.cfg.Gerrit.Email {
					return false
				}
			}

			return true
		}()
		if !canBenchmark {
			continue
		}

		priority := 0
		if presubmitReady {
			priority += 10
		}
		priority += codeReviewScore
		if codeReviewApproved {
			priority += 2
		}
		if kokoroApproved {
			priority++
		}

		candidates = append(candidates, candidate{change, priority})
	}

	// Sort the candidates
	sort.Slice(candidates, func(i, j int) bool {
		return candidates[i].priority > candidates[j].priority
	})

	if len(candidates) > 0 {
		log.Printf("%d gerrit changes to benchmark", len(candidates))
		return &candidates[0].change, nil
	}
	return nil, nil
}

// benchmarks the gerrit change, posting the findings to the change
func (e env) benchmarkGerritChange(change gerrit.ChangeInfo) error {
	current := change.Revisions[change.CurrentRevision]
	log.Printf("fetching '%v'...", current.Ref)
	currentHash, err := e.dawnRepo.Fetch(current.Ref, &git.FetchOptions{
		Credentials: e.cfg.Dawn.Credentials,
	})
	if err != nil {
		return err
	}
	parent := current.Commit.Parents[0].Commit
	parentHash, err := git.ParseHash(parent)
	if err != nil {
		return fmt.Errorf("failed to parse parent hash '%v':\n  %v", parent, err)
	}

	postMsg := func(notify, msg string) error {
		_, _, err = e.gerrit.Changes.SetReview(change.ChangeID, currentHash.String(), &gerrit.ReviewInput{
			Message: msg,
			Tag:     "autogenerated:perfmon",
			Notify:  notify,
		})
		if err != nil {
			return fmt.Errorf("failed to post message to gerrit change:\n  %v", err)
		}
		return nil
	}

	newRun, err := e.benchmarkTintChange(currentHash)
	if err != nil {
		log.Printf("ERROR: %v", err)
		buildErr := errFailedToBuild{}
		if errors.As(err, &buildErr) {
			return postMsg("OWNER", fmt.Sprintf("patchset %v failed to build", current.Number))
		}
		benchErr := errFailedToBenchmark{}
		if errors.As(err, &benchErr) {
			return postMsg("OWNER", fmt.Sprintf("patchset %v failed to benchmark", current.Number))
		}
		return err
	}
	if _, err := e.dawnRepo.Fetch(parent, &git.FetchOptions{
		Credentials: e.cfg.Dawn.Credentials,
	}); err != nil {
		return err
	}
	parentRun, err := e.benchmarkTintChangeIfNotCached(parentHash)
	if err != nil {
		return err
	}

	const minDiff = time.Microsecond * 50 // Ignore time diffs less than this duration
	const minRelDiff = 0.01               // Ignore absolute relative diffs between [1, 1+x]
	diff := bench.Compare(parentRun.Benchmarks, newRun.Benchmarks, minDiff, minRelDiff)
	diffFmt := bench.DiffFormat{
		TestName:        true,
		Delta:           true,
		PercentChangeAB: true,
		TimeA:           true,
		TimeB:           true,
	}

	msg := &strings.Builder{}
	fmt.Fprintln(msg, "Perfmon analysis:")
	fmt.Fprintln(msg)
	fmt.Fprintln(msg, "```")
	fmt.Fprintf(msg, "A: parent change (%v) -> B: patchset %v\n", parent[:7], current.Number)
	fmt.Fprintln(msg)
	for _, line := range strings.Split(diff.Format(diffFmt), "\n") {
		fmt.Fprintf(msg, "  %v\n", line)
	}
	fmt.Fprintln(msg, "```")

	notify := "OWNER"
	if len(diff) > 0 {
		notify = "OWNER_REVIEWERS"
	}
	return postMsg(notify, msg.String())
}

// waitForTempsToSettle waits for the maximum temperature of all sensors to drop
// below the threshold value specified by the config.
func (e env) waitForTempsToSettle() error {
	if e.cfg.CPUTempSensorName == "" {
		time.Sleep(time.Second * 30)
		return nil
	}
	const timeout = 5 * time.Minute
	start := time.Now()
	for {
		temp, err := maxTemp(e.cfg.CPUTempSensorName)
		if err != nil {
			return fmt.Errorf("failed to obtain system temeratures: %v", err)
		}
		if temp < e.cfg.BenchmarkMaxTemp {
			log.Printf("temperatures settled. current: %v°C", temp)
			return nil
		}
		if time.Since(start) > timeout {
			log.Printf("timeout waiting for temperatures to settle. current: %v°C", temp)
			return nil
		}
		log.Printf("waiting for temperatures to settle. current: %v°C, max: %v°C", temp, e.cfg.BenchmarkMaxTemp)
		time.Sleep(time.Second * 10)
	}
}

// createOrOpenGitRepo creates a new local repo by cloning cfg.URL into
// filepath, or opens the existing repo at filepath.
func createOrOpenGitRepo(g *git.Git, filepath string, cfg GitConfig) (*git.Repository, error) {
	repo, err := g.Open(filepath)
	if errors.Is(err, git.ErrRepositoryDoesNotExist) {
		log.Printf("cloning '%v' branch '%v' to '%v'...", cfg.URL, cfg.Branch, filepath)
		repo, err = g.Clone(filepath, cfg.URL, &git.CloneOptions{
			Branch:      cfg.Branch,
			Credentials: cfg.Credentials,
		})
	}
	if err != nil {
		return nil, fmt.Errorf("failed to open git repository '%v':\n  %w", filepath, err)
	}
	return repo, err
}

// loadConfig loads the perfmon config file.
func loadConfig(path string) (Config, error) {
	f, err := os.Open(path)
	if err != nil {
		return Config{}, fmt.Errorf("failed to open config file at '%v':\n  %w", path, err)
	}
	cfg := Config{}
	if err := json.NewDecoder(f).Decode(&cfg); err != nil {
		return Config{}, fmt.Errorf("failed to load config file at '%v':\n  %w", path, err)
	}
	cfg.setDefaults()
	return cfg, nil
}

// makeWorkingDirs creates the dawn repo and results repo directories.
func makeWorkingDirs(cfg Config) (dawnDir, resultsDir string, err error) {
	wd, err := expandHomeDir(cfg.WorkingDir)
	if err != nil {
		return "", "", err
	}
	if err := os.MkdirAll(wd, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create working directory '%v':\n  %w", wd, err)
	}
	dawnDir = filepath.Join(wd, "dawn")
	if err := os.MkdirAll(dawnDir, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create working dawn directory '%v':\n  %w", dawnDir, err)
	}
	resultsDir = filepath.Join(wd, "results")
	if err := os.MkdirAll(resultsDir, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create working results directory '%v':\n  %w", resultsDir, err)
	}
	return dawnDir, resultsDir, nil
}

// fetchAndCheckoutLatest calls fetch(cfg.Branch) followed by checkoutLatest().
func fetchAndCheckoutLatest(repo *git.Repository, cfg GitConfig) error {
	hash, err := repo.Fetch(cfg.Branch, &git.FetchOptions{
		Credentials: cfg.Credentials,
	})
	if err != nil {
		return err
	}
	if err := repo.Checkout(hash.String(), nil); err != nil {
		return err
	}
	return checkout(hash, repo)
}

// checkout checks out the change with the given hash.
// Note: call fetch() to ensure that this is the latest change on the
// branch.
func checkout(hash git.Hash, repo *git.Repository) error {
	if err := repo.Checkout(hash.String(), nil); err != nil {
		return fmt.Errorf("failed to checkout '%v':\n  %w", hash, err)
	}
	return nil
}

// expandHomeDir returns path with all occurrences of '~' replaced with the user
// home directory.
func expandHomeDir(path string) (string, error) {
	if strings.ContainsRune(path, '~') {
		home, err := os.UserHomeDir()
		if err != nil {
			return "", fmt.Errorf("failed to expand home dir:\n  %w", err)
		}
		path = strings.ReplaceAll(path, "~", home)
	}
	return path, nil
}

// tools holds the file paths to the executables used by this tool
var tools struct {
	ccache  string
	cmake   string
	gclient string
	git     string
	ninja   string
	sensors string
}

// findTools looks for the file paths for executables used by this tool,
// returning an error if any could not be found.
func findTools() error {
	for _, tool := range []struct {
		name string
		path *string
	}{
		{"ccache", &tools.ccache},
		{"cmake", &tools.cmake},
		{"gclient", &tools.gclient},
		{"git", &tools.git},
		{"ninja", &tools.ninja},
		{"sensors", &tools.sensors},
	} {
		path, err := exec.LookPath(tool.name)
		if err != nil {
			return fmt.Errorf("failed to find path to '%v':\n  %w", tool.name, err)
		}
		*tool.path = path
	}
	return nil
}

// copyFile copies the file at srcPath to dstPath.
func copyFile(dstPath, srcPath string) error {
	src, err := os.Open(srcPath)
	if err != nil {
		return fmt.Errorf("failed to open file '%v':\n  %w", srcPath, err)
	}
	defer src.Close()
	dst, err := os.Create(dstPath)
	if err != nil {
		return fmt.Errorf("failed to create file '%v':\n  %w", dstPath, err)
	}
	defer dst.Close()
	_, err = io.Copy(dst, src)
	return err
}

// The regular expression to parse a temperature from 'sensors'
var reTemp = regexp.MustCompile("([0-9]+.[0-9])°C")

// maxTemp returns the maximum sensor temperature in celsius returned by 'sensors'
func maxTemp(sensorName string) (float32, error) {
	output, err := call(tools.sensors, "", time.Second*2, sensorName)
	if err != nil {
		return 0, err
	}
	var maxTemp float32
	for _, match := range reTemp.FindAllStringSubmatch(output, -1) {
		var temp float32
		if _, err := fmt.Sscanf(match[1], "%f", &temp); err == nil {
			if temp > maxTemp {
				maxTemp = temp
			}
		}
	}
	return maxTemp, nil
}

// call invokes the executable exe in the current working directory wd, with
// the provided arguments.
// If the executable does not complete within the timeout duration, then an
// error is returned.
func call(exe, wd string, timeout time.Duration, args ...string) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	cmd := exec.CommandContext(ctx, exe, args...)
	cmd.Dir = wd
	out, err := cmd.CombinedOutput()
	if err != nil {
		return string(out), fmt.Errorf("'%v %v' failed:\n  %w\n%v", exe, args, err, string(out))
	}
	return string(out), nil
}

// hash returns a hash of the string representation of 'o'.
func hash(o interface{}) string {
	str := fmt.Sprintf("%+v", o)
	hash := sha256.New()
	hash.Write([]byte(str))
	return hex.EncodeToString(hash.Sum(nil))[:8]
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func max(a, b int) int {
	if a > b {
		return a
	}
	return b
}
