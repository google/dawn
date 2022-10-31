// Copyright 2022 The Dawn Authors
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

package common

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"dawn.googlesource.com/dawn/tools/src/subcmd"
	"go.chromium.org/luci/auth"
	rdbpb "go.chromium.org/luci/resultdb/proto/v1"
)

// ResultSource describes the source of CTS test results.
// ResultSource is commonly used by command line flags for specifying from
// where the results should be loaded / fetched.
// If neither File or Patchset are specified, then results will be fetched from
// the last successful CTS roll.
type ResultSource struct {
	// The directory used to cache results fetched from ResultDB
	CacheDir string
	// If specified, results will be loaded from this file path
	// Must not be specified if Patchset is also specified.
	File string
	// If specified, results will be fetched from this gerrit patchset
	// Must not be specified if File is also specified.
	Patchset gerrit.Patchset
}

// RegisterFlags registers the ResultSource fields as commandline flags for use
// by command line tools.
func (r *ResultSource) RegisterFlags(cfg Config) {
	flag.StringVar(&r.CacheDir, "cache", DefaultCacheDir, "path to the results cache")
	flag.StringVar(&r.File, "results", "", "local results.txt file (mutually exclusive with --cl)")
	r.Patchset.RegisterFlags(cfg.Gerrit.Host, cfg.Gerrit.Project)
}

// GetResults loads or fetches the results, based on the values of r.
// GetResults will update the ResultSource with the inferred patchset, if a file
// and specific patchset was not specified.
func (r *ResultSource) GetResults(ctx context.Context, cfg Config, auth auth.Options) (result.List, error) {
	// Check that File and Patchset weren't both specified
	ps := &r.Patchset
	if r.File != "" && ps.Change != 0 {
		fmt.Fprintln(flag.CommandLine.Output(), "only one of --results and --cl can be specified")
		return nil, subcmd.ErrInvalidCLA
	}

	// If a file was specified, then load that.
	if r.File != "" {
		return result.Load(r.File)
	}

	// Initialize the buildbucket and resultdb clients
	bb, err := buildbucket.New(ctx, auth)
	if err != nil {
		return nil, err
	}
	rdb, err := resultsdb.New(ctx, auth)
	if err != nil {
		return nil, err
	}

	// If no change was specified, then pull the results from the most recent
	// CTS roll.
	if ps.Change == 0 {
		fmt.Println("no change specified, scanning gerrit for last CTS roll...")
		gerrit, err := gerrit.New(cfg.Gerrit.Host, gerrit.Credentials{})
		if err != nil {
			return nil, err
		}
		latest, err := LatestCTSRoll(gerrit)
		if err != nil {
			return nil, err
		}
		fmt.Printf("scanning for latest patchset of %v...\n", latest.Number)
		var results result.List
		results, *ps, err = MostRecentResultsForChange(ctx, cfg, r.CacheDir, gerrit, bb, rdb, latest.Number)
		if err != nil {
			return nil, err
		}
		fmt.Printf("using results from cl %v ps %v...\n", ps.Change, ps.Patchset)
		return results, nil
	}

	// If a change, but no patchset was specified, then query the most recent
	// patchset.
	if ps.Patchset == 0 {
		gerrit, err := gerrit.New(cfg.Gerrit.Host, gerrit.Credentials{})
		if err != nil {
			return nil, err
		}
		*ps, err = gerrit.LatestPatchest(strconv.Itoa(ps.Change))
		if err != nil {
			err := fmt.Errorf("failed to find latest patchset of change %v: %w",
				ps.Change, err)
			return nil, err
		}
	}

	// Obtain the patchset's results, kicking a build if there are no results
	// already available.
	log.Printf("fetching results from cl %v ps %v...", ps.Change, ps.Patchset)
	builds, err := GetOrStartBuildsAndWait(ctx, cfg, *ps, bb, false)
	if err != nil {
		return nil, err
	}

	results, err := CacheResults(ctx, cfg, *ps, r.CacheDir, rdb, builds)
	if err != nil {
		return nil, err
	}

	return results, nil
}

// CacheResults looks in the cache at 'cacheDir' for the results for the given
// patchset. If the cache contains the results, then these are loaded and
// returned. If the cache does not contain the results, then they are fetched
// using GetResults(), saved to the cache directory and are returned.
func CacheResults(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	cacheDir string,
	rdb *resultsdb.ResultsDB,
	builds BuildsByName) (result.List, error) {

	var cachePath string
	if cacheDir != "" {
		dir := fileutils.ExpandHome(cacheDir)
		path := filepath.Join(dir, strconv.Itoa(ps.Change), fmt.Sprintf("ps-%v.txt", ps.Patchset))
		if _, err := os.Stat(path); err == nil {
			return result.Load(path)
		}
		cachePath = path
	}

	results, err := GetResults(ctx, cfg, rdb, builds)
	if err != nil {
		return nil, err
	}

	if err := result.Save(cachePath, results); err != nil {
		log.Println("failed to save results to cache: %w", err)
	}

	return results, nil
}

// GetResults fetches the build results from ResultDB.
// GetResults does not trigger new builds.
func GetResults(
	ctx context.Context,
	cfg Config,
	rdb *resultsdb.ResultsDB,
	builds BuildsByName) (result.List, error) {

	fmt.Printf("fetching results from resultdb...")

	lastPrintedDot := time.Now()

	toStatus := func(s rdbpb.TestStatus) result.Status {
		switch s {
		default:
			return result.Unknown
		case rdbpb.TestStatus_PASS:
			return result.Pass
		case rdbpb.TestStatus_FAIL:
			return result.Failure
		case rdbpb.TestStatus_CRASH:
			return result.Crash
		case rdbpb.TestStatus_ABORT:
			return result.Abort
		case rdbpb.TestStatus_SKIP:
			return result.Skip
		}
	}

	results := result.List{}
	err := rdb.QueryTestResults(ctx, builds.ids(), cfg.Test.Prefix+".*", func(rpb *rdbpb.TestResult) error {
		if time.Since(lastPrintedDot) > 5*time.Second {
			lastPrintedDot = time.Now()
			fmt.Printf(".")
		}

		if !strings.HasPrefix(rpb.GetTestId(), cfg.Test.Prefix) {
			return nil
		}

		testName := rpb.GetTestId()[len(cfg.Test.Prefix):]
		status := toStatus(rpb.Status)
		tags := result.NewTags()

		duration := rpb.GetDuration().AsDuration()
		mayExonerate := false

		for _, sp := range rpb.Tags {
			if sp.Key == "typ_tag" {
				tags.Add(sp.Value)
			}
			if sp.Key == "javascript_duration" {
				var err error
				if duration, err = time.ParseDuration(sp.Value); err != nil {
					return err
				}
			}
			if sp.Key == "may_exonerate" {
				var err error
				if mayExonerate, err = strconv.ParseBool(sp.Value); err != nil {
					return err
				}
			}
		}

		if status == result.Pass && duration > cfg.Test.SlowThreshold {
			status = result.Slow
		}

		results = append(results, result.Result{
			Query:        query.Parse(testName),
			Status:       status,
			Tags:         tags,
			Duration:     duration,
			MayExonerate: mayExonerate,
		})

		return nil
	})

	fmt.Println(" done")

	if err != nil {
		return nil, err
	}

	// Expand aliased tags, remove specific tags
	CleanTags(cfg, &results)

	results.Sort()
	return results, err
}

// LatestCTSRoll returns for the latest merged CTS roll that landed in the past
// month. If no roll can be found, then an error is returned.
func LatestCTSRoll(g *gerrit.Gerrit) (gerrit.ChangeInfo, error) {
	changes, _, err := g.QueryChanges(
		`status:merged`,
		`-age:1month`,
		fmt.Sprintf(`message:"%v"`, RollSubjectPrefix))
	if err != nil {
		return gerrit.ChangeInfo{}, err
	}
	if len(changes) == 0 {
		return gerrit.ChangeInfo{}, fmt.Errorf("no change found")
	}
	sort.Slice(changes, func(i, j int) bool {
		return changes[i].Submitted.Time.After(changes[j].Submitted.Time)
	})
	return changes[0], nil
}

// LatestPatchset returns the most recent patchset for the given change.
func LatestPatchset(g *gerrit.Gerrit, change int) (gerrit.Patchset, error) {
	ps, err := g.LatestPatchest(strconv.Itoa(change))
	if err != nil {
		err := fmt.Errorf("failed to find latest patchset of change %v: %w",
			ps.Change, err)
		return gerrit.Patchset{}, err
	}
	return ps, nil
}

// MostRecentResultsForChange returns the results from the most recent patchset
// that has build results. If no results can be found for the entire change,
// then an error is returned.
func MostRecentResultsForChange(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	g *gerrit.Gerrit,
	bb *buildbucket.Buildbucket,
	rdb *resultsdb.ResultsDB,
	change int) (result.List, gerrit.Patchset, error) {

	ps, err := LatestPatchset(g, change)
	if err != nil {
		return nil, gerrit.Patchset{}, nil
	}

	for ps.Patchset > 0 {
		builds, err := GetBuilds(ctx, cfg, ps, bb)
		if err != nil {
			return nil, gerrit.Patchset{}, err
		}
		if len(builds) > 0 {
			if err := WaitForBuildsToComplete(ctx, cfg, ps, bb, builds); err != nil {
				return nil, gerrit.Patchset{}, err
			}

			results, err := CacheResults(ctx, cfg, ps, cacheDir, rdb, builds)
			if err != nil {
				return nil, gerrit.Patchset{}, err
			}

			if len(results) > 0 {
				return results, ps, nil
			}
		}
		ps.Patchset--
	}

	return nil, gerrit.Patchset{}, fmt.Errorf("no builds found for change %v", change)
}

// CleanTags modifies each result so that tags in cfg.Tag.Remove are removed and
// duplicate results are removed by erring towards Failure.
// See: crbug.com/dawn/1387, crbug.com/dawn/1401
func CleanTags(cfg Config, results *result.List) {
	// Remove any tags found in cfg.Tag.Remove
	remove := result.NewTags(cfg.Tag.Remove...)
	for _, r := range *results {
		r.Tags.RemoveAll(remove)
	}
	// Clean up duplicate results
	*results = results.ReplaceDuplicates(func(s result.Statuses) result.Status {
		// If all results have the same status, then use that.
		if len(s) == 1 {
			return s.One()
		}
		// Mixed statuses. Replace with something appropriate.
		switch {
		case s.Contains(result.Crash):
			return result.Crash
		case s.Contains(result.Abort):
			return result.Abort
		case s.Contains(result.Failure):
			return result.Failure
		case s.Contains(result.Slow):
			return result.Slow
		}
		return result.Failure
	})
}
