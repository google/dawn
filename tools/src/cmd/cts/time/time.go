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

package time

import (
	"context"
	"flag"
	"fmt"
	"math"
	"sort"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/subcmd"
	"go.chromium.org/luci/auth/client/authcli"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		source    common.ResultSource
		auth      authcli.Flags
		tags      string
		query     string
		aggregate bool
		topN      int
		histogram bool
	}
}

func (cmd) Name() string {
	return "time"
}

func (cmd) Desc() string {
	return "displays timing information for tests"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.source.RegisterFlags(cfg)
	c.flags.auth.Register(flag.CommandLine, common.DefaultAuthOptions())
	flag.IntVar(&c.flags.topN, "top", 0, "print the top N slowest tests")
	flag.BoolVar(&c.flags.histogram, "histogram", false, "print a histogram of test timings")
	flag.StringVar(&c.flags.query, "query", "", "test query to filter results")
	flag.StringVar(&c.flags.tags, "tags", "", "comma-separated list of tags to filter results")
	flag.BoolVar(&c.flags.aggregate, "aggregate", false, "aggregate times by test")
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Validate command line arguments
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	// Obtain the results
	results, err := c.flags.source.GetResults(ctx, cfg, auth)
	if err != nil {
		return err
	}

	if len(results) == 0 {
		return fmt.Errorf("no results found")
	}

	// If tags were provided, filter the results to those that contain these tags
	if c.flags.tags != "" {
		results = results.FilterByTags(result.StringToTags(c.flags.tags))
		if len(results) == 0 {
			return fmt.Errorf("no results after filtering by tags")
		}
	}

	if c.flags.query != "" {
		results = results.FilterByQuery(query.Parse(c.flags.query))
		if len(results) == 0 {
			return fmt.Errorf("no results after filtering by test query")
		}
	}

	if c.flags.aggregate {
		type Key struct {
			Query  query.Query
			Status result.Status
			Tags   string
		}
		merged := map[Key]result.Result{}
		for _, r := range results {
			k := Key{
				Query: query.Query{
					Suite: r.Query.Suite,
					Files: r.Query.Files,
					Tests: r.Query.Tests,
					Cases: "*",
				},
				Status: r.Status,
				Tags:   result.TagsToString(r.Tags),
			}
			entry, exists := merged[k]
			if exists {
				entry.Duration += r.Duration
			} else {
				entry = result.Result{
					Query:    k.Query,
					Duration: r.Duration,
					Status:   r.Status,
					Tags:     r.Tags,
				}
			}
			merged[k] = entry
		}

		results = result.List{}
		for _, r := range merged {
			results = append(results, r)
		}
	}

	// Sort the results with longest duration first
	sort.Slice(results, func(i, j int) bool {
		return results[i].Duration > results[j].Duration
	})

	didSomething := false

	// Did the user request --top N ?
	if c.flags.topN > 0 {
		didSomething = true
		topN := results
		if c.flags.topN < len(results) {
			topN = topN[:c.flags.topN]
		}
		for i, r := range topN {
			fmt.Printf("%3.1d: %v\n", i, r)
		}
	}

	// Did the user request --histogram ?
	if c.flags.histogram {
		maxTime := results[0].Duration

		const (
			numBins = 25
			pow     = 2.0
		)

		binToDuration := func(i int) time.Duration {
			frac := math.Pow(float64(i)/float64(numBins), pow)
			return time.Duration(float64(maxTime) * frac)
		}
		durationToBin := func(d time.Duration) int {
			frac := math.Pow(float64(d)/float64(maxTime), 1.0/pow)
			idx := int(frac * numBins)
			if idx >= numBins-1 {
				return numBins - 1
			}
			return idx
		}

		didSomething = true
		bins := make([]int, numBins)
		for _, r := range results {
			idx := durationToBin(r.Duration)
			bins[idx] = bins[idx] + 1
		}
		for i, bin := range bins {
			fmt.Printf("[%.8v, %.8v]: %v\n", binToDuration(i), binToDuration(i+1), bin)
		}
	}

	// If the user didn't request anything, show a helpful message
	if !didSomething {
		fmt.Fprintln(flag.CommandLine.Output(), "no action flags specified for", c.Name())
		fmt.Fprintln(flag.CommandLine.Output())
		flag.Usage()
		return subcmd.ErrInvalidCLA
	}

	return nil
}
