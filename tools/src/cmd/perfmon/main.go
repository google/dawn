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
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
	"sort"
	"strings"
	"time"

	"dawn.googlesource.com/tint/tools/src/bench"
	"dawn.googlesource.com/tint/tools/src/git"
	"github.com/andygrunwald/go-gerrit"
	"github.com/go-git/go-git/v5/plumbing/transport"
	"github.com/go-git/go-git/v5/plumbing/transport/http"
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

	tintDir, resultsDir, err := makeWorkingDirs(cfg)
	if err != nil {
		return err
	}
	tintRepo, err := createOrOpenGitRepo(g, tintDir, cfg.Tint)
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

	e := env{
		cfg:         cfg,
		git:         g,
		system:      sysInfo,
		systemID:    hash(sysInfo)[:8],
		tintDir:     tintDir,
		buildDir:    filepath.Join(tintDir, "out"),
		resultsDir:  resultsDir,
		tintRepo:    tintRepo,
		resultsRepo: resultsRepo,
		gerrit:      gerritClient,

		benchmarkCache: map[git.Hash]*bench.Run{},
	}

	for true {
		didSomething, err := e.doSomeWork()
		if err != nil {
			log.Printf("ERROR: %v", err)
			time.Sleep(time.Minute * 10)
			continue
		}
		if !didSomething {
			log.Println("nothing to do. Sleeping...")
			time.Sleep(time.Minute * 5)
		}
	}

	return nil
}

// Config holds the root configuration options for the perfmon tool
type Config struct {
	WorkingDir           string
	RootChange           git.Hash
	Tint                 GitConfig
	Results              GitConfig
	Gerrit               GerritConfig
	Timeouts             TimeoutsConfig
	ExternalAccounts     []string
	BenchmarkRepetitions int
}

// GitConfig holds the configuration options for accessing a git repo
type GitConfig struct {
	URL    string
	Branch string
	Auth   git.Auth
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

// CommitResults holds the results of a single tint commit
type CommitResults struct {
	Commit            string
	CommitTime        time.Time
	CommitDescription string
	Benchmarks        []Benchmark
}

// Benchmark holds the benchmark results for a single test
type Benchmark struct {
	Name   string
	Mean   float64
	Median float64
	Stddev float64
}

// setDefaults assigns default values to unassigned fields of cfg
func (cfg *Config) setDefaults() {
	if cfg.RootChange.IsZero() {
		cfg.RootChange, _ = git.ParseHash("be2362b18c792364c6bf5744db6d3837fbc655a0")
	}
	cfg.Tint.setDefaults()
	cfg.Results.setDefaults()
	cfg.Timeouts.setDefaults()
	if cfg.BenchmarkRepetitions < 2 {
		cfg.BenchmarkRepetitions = 2
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

// AuthConfig holds the authentication options for accessing a git repo
type AuthConfig struct {
	Username string
	Password string
}

// authMethod returns a http.BasicAuth constructed from the AuthConfig
func (cfg AuthConfig) authMethod() transport.AuthMethod {
	if cfg.Username != "" || cfg.Password != "" {
		return &http.BasicAuth{Username: cfg.Username, Password: cfg.Password}
	}
	return nil
}

// env holds the perfmon main environment state
type env struct {
	cfg         Config
	git         *git.Git
	system      []cpu.InfoStat
	systemID    string
	tintDir     string
	buildDir    string
	resultsDir  string
	tintRepo    *git.Repository
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
			log.Printf("benchmarking %v changes...", len(changesToBenchmark))
			for i, c := range changesToBenchmark {
				log.Printf("benchmarking %v/%v....", i+1, len(changesToBenchmark))
				benchRes, err := e.benchmarkTintChange(c)
				if err != nil {
					return true, err
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
	return false, nil
}

// changesToBenchmark fetches the list of changes that do not currently have
// benchmark results, which should be benchmarked.
func (e env) changesToBenchmark() ([]git.Hash, error) {
	log.Println("syncing tint repo...")
	latest, err := e.tintRepo.Fetch(e.cfg.Tint.Branch, &git.FetchOptions{Auth: e.cfg.Tint.Auth})
	if err != nil {
		return nil, err
	}
	allChanges, err := e.tintRepo.Log(&git.LogOptions{
		From: e.cfg.RootChange.String(),
		To:   latest.String(),
	})
	if err != nil {
		return nil, fmt.Errorf("failed to obtain tint log:\n  %w", err)
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

// benchmarkTintChange checks out the given commit, fetches the tint third party
// dependencies, builds tint, then runs the benchmarks, returning the results.
func (e env) benchmarkTintChange(hash git.Hash) (*bench.Run, error) {
	if cached, ok := e.benchmarkCache[hash]; ok {
		log.Printf("reusing cached benchmark results of '%v'...", hash)
		return cached, nil
	}

	log.Printf("checking out tint at '%v'...", hash)
	if err := checkout(hash, e.tintRepo); err != nil {
		return nil, err
	}
	log.Println("fetching tint dependencies...")
	if err := e.fetchTintDeps(); err != nil {
		return nil, err
	}
	log.Println("building tint...")
	if err := e.buildTint(); err != nil {
		return nil, err
	}
	log.Println("benchmarking tint...")
	run, err := e.benchmarkTint()
	if err != nil {
		return nil, err
	}

	e.benchmarkCache[hash] = run
	return run, nil
}

// benchmarksToCommitResults converts the benchmarks in the provided bench.Run
// to a CommitResults.
func (e env) benchmarksToCommitResults(hash git.Hash, results bench.Run) (*CommitResults, error) {
	commits, err := e.tintRepo.Log(&git.LogOptions{
		From:  hash.String(),
		Count: 1,
	})
	if err != nil || len(commits) != 1 {
		return nil, fmt.Errorf("failed to get commit object '%v' of tint repo:\n  %w", hash, err)
	}
	commit := commits[0]

	m := map[string]Benchmark{}
	for _, b := range results.Benchmarks {
		benchmark := m[b.Name]
		benchmark.Name = b.Name
		switch b.AggregateType {
		case bench.Mean:
			benchmark.Mean = float64(b.Duration) / float64(time.Second)
		case bench.Median:
			benchmark.Median = float64(b.Duration) / float64(time.Second)
		case bench.Stddev:
			benchmark.Stddev = float64(b.Duration) / float64(time.Second)
		}
		m[b.Name] = benchmark
	}

	sorted := make([]Benchmark, 0, len(m))
	for _, b := range m {
		sorted = append(sorted, b)
	}
	sort.Slice(sorted, func(i, j int) bool { return sorted[i].Name < sorted[i].Name })

	return &CommitResults{
		Commit:            commit.Hash.String(),
		CommitDescription: commit.Subject,
		CommitTime:        commit.Date,
		Benchmarks:        sorted,
	}, nil
}

// changesWithBenchmarks returns a set of tint changes that we already have
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

	h.Commits = append(h.Commits, res)

	// Sort the commits by timestamp
	sort.Slice(h.Commits, func(i, j int) bool { return h.Commits[i].CommitTime.Before(h.Commits[j].CommitTime) })

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
	if err := e.resultsRepo.Push(hash.String(), e.cfg.Results.Branch, &git.PushOptions{Auth: e.cfg.Results.Auth}); err != nil {
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

// fetchTintDeps fetches the third party tint dependencies using gclient.
func (e env) fetchTintDeps() error {
	gclientConfig := filepath.Join(e.tintDir, ".gclient")
	if _, err := os.Stat(gclientConfig); errors.Is(err, os.ErrNotExist) {
		standalone := filepath.Join(e.tintDir, "standalone.gclient")
		if err := copyFile(gclientConfig, standalone); err != nil {
			return fmt.Errorf("failed to copy '%v' to '%v':\n  %w", standalone, gclientConfig, err)
		}
	}
	if _, err := call(tools.gclient, e.tintDir, e.cfg.Timeouts.Sync,
		"sync",
		"--force",
	); err != nil {
		return fmt.Errorf("failed to fetch tint dependencies:\n  %w", err)
	}
	return nil
}

// buildTint builds the tint benchmarks.
func (e env) buildTint() error {
	if err := os.MkdirAll(e.buildDir, 0777); err != nil {
		return fmt.Errorf("failed to create build directory at '%v':\n  %w", e.buildDir, err)
	}
	if _, err := call(tools.cmake, e.buildDir, e.cfg.Timeouts.Build,
		e.tintDir,
		"-GNinja",
		"-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
		"-DCMAKE_BUILD_TYPE=Release",
		"-DTINT_BUILD_SPV_READER=1",
		"-DTINT_BUILD_WGSL_READER=1",
		"-DTINT_BUILD_GLSL_WRITER=1",
		"-DTINT_BUILD_HLSL_WRITER=1",
		"-DTINT_BUILD_MSL_WRITER=1",
		"-DTINT_BUILD_SPV_WRITER=1",
		"-DTINT_BUILD_WGSL_WRITER=1",
		"-DTINT_BUILD_BENCHMARKS=1",
	); err != nil {
		return errFailedToBuild{fmt.Errorf("failed to generate tint build config:\n  %w", err)}
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

// benchmarkTint runs the tint benchmarks, returning the results.
func (e env) benchmarkTint() (*bench.Run, error) {
	exe := filepath.Join(e.buildDir, "tint-benchmark")
	out, err := call(exe, e.buildDir, e.cfg.Timeouts.Benchmark,
		"--benchmark_format=json",
		fmt.Sprintf("--benchmark_repetitions=%v", e.cfg.BenchmarkRepetitions),
	)
	if err != nil {
		return nil, fmt.Errorf("failed to benchmark tint:\n  %w", err)
	}

	results, err := bench.Parse(out)
	if err != nil {
		return nil, fmt.Errorf("failed to parse benchmark results:\n  %w", err)
	}
	return &results, nil
}

// findGerritChangeToBenchmark queries gerrit for a change to benchmark.
func (e env) findGerritChangeToBenchmark() (*gerrit.ChangeInfo, error) {
	log.Println("querying gerrit for changes...")
	results, _, err := e.gerrit.Changes.QueryChanges(&gerrit.QueryChangeOptions{
		QueryOptions: gerrit.QueryOptions{
			Query: []string{"project:tint status:open+-age:3d"},
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
					if strings.ToLower(current.Commit.Committer.Email) == strings.ToLower(email) {
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
	currentHash, err := e.tintRepo.Fetch(current.Ref, &git.FetchOptions{Auth: e.cfg.Tint.Auth})
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
		var ftb errFailedToBuild
		if errors.As(err, &ftb) {
			return postMsg("OWNER", fmt.Sprintf("patchset %v failed to build", current.Number))
		}
		return err
	}
	if _, err := e.tintRepo.Fetch(parent, &git.FetchOptions{Auth: e.cfg.Tint.Auth}); err != nil {
		return err
	}
	parentRun, err := e.benchmarkTintChange(parentHash)
	if err != nil {
		return err
	}

	// filters the benchmark results to only the mean aggregate values
	meanBenchmarkResults := func(in []bench.Benchmark) []bench.Benchmark {
		out := make([]bench.Benchmark, 0, len(in))
		for _, b := range in {
			if b.AggregateType == bench.Mean {
				out = append(out, b)
			}
		}
		return out
	}

	newResults := meanBenchmarkResults(newRun.Benchmarks)
	parentResults := meanBenchmarkResults(parentRun.Benchmarks)

	const minDiff = time.Microsecond * 50 // Ignore time diffs less than this duration
	const minRelDiff = 0.01               // Ignore absolute relative diffs between [1, 1+x]
	diff := bench.Compare(parentResults, newResults, minDiff, minRelDiff)
	diffFmt := bench.DiffFormat{
		TestName:        true,
		Delta:           true,
		PercentChangeAB: true,
		TimeA:           true,
		TimeB:           true,
	}

	msg := &strings.Builder{}
	fmt.Fprintf(msg, "Tint perfmon analysis:\n")
	fmt.Fprintf(msg, "  \n")
	fmt.Fprintf(msg, "  A: parent change (%v) -> B: patchset %v\n", parent[:7], current.Number)
	fmt.Fprintf(msg, "  \n")
	for _, line := range strings.Split(diff.Format(diffFmt), "\n") {
		fmt.Fprintf(msg, "  %v\n", line)
	}

	notify := "OWNER"
	if len(diff) > 0 {
		notify = "OWNER_REVIEWERS"
	}
	return postMsg(notify, msg.String())
}

// createOrOpenGitRepo creates a new local repo by cloning cfg.URL into
// filepath, or opens the existing repo at filepath.
func createOrOpenGitRepo(g *git.Git, filepath string, cfg GitConfig) (*git.Repository, error) {
	repo, err := g.Open(filepath)
	if errors.Is(err, git.ErrRepositoryDoesNotExist) {
		log.Printf("cloning '%v' branch '%v' to '%v'...", cfg.URL, cfg.Branch, filepath)
		repo, err = g.Clone(filepath, cfg.URL, &git.CloneOptions{
			Branch: cfg.Branch,
			Auth:   cfg.Auth,
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

// makeWorkingDirs builds the tint repo and results repo directories.
func makeWorkingDirs(cfg Config) (tintDir, resultsDir string, err error) {
	wd, err := expandHomeDir(cfg.WorkingDir)
	if err != nil {
		return "", "", err
	}
	if err := os.MkdirAll(wd, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create working directory '%v':\n  %w", wd, err)
	}
	tintDir = filepath.Join(wd, "tint")
	if err := os.MkdirAll(tintDir, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create working tint directory '%v':\n  %w", tintDir, err)
	}
	resultsDir = filepath.Join(wd, "results")
	if err := os.MkdirAll(resultsDir, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create working results directory '%v':\n  %w", resultsDir, err)
	}
	return tintDir, resultsDir, nil
}

// fetchAndCheckoutLatest calls fetch(cfg.Branch) followed by checkoutLatest().
func fetchAndCheckoutLatest(repo *git.Repository, cfg GitConfig) error {
	hash, err := repo.Fetch(cfg.Branch, &git.FetchOptions{Auth: cfg.Auth})
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
