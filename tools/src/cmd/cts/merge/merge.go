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

package merge

import (
	"context"
	"flag"
	"fmt"
	"os"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		output string
	}
}

func (cmd) Name() string { return "merge" }

func (cmd) Desc() string { return "merges results files into one" }

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	flag.StringVar(&c.flags.output, "o", "results.txt", "output file. '-' writes to stdout")
	return []string{"first-results.txt", "second-results.txt ..."}, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Load each of the results files and merge together
	var results result.List
	for _, path := range flag.Args() {
		// Load results
		r, err := result.Load(path)
		if err != nil {
			return fmt.Errorf("while reading '%v': %w", path, err)
		}
		// Combine and merge
		results = result.Merge(results, r)
	}

	// Open output file
	output := os.Stdout
	if c.flags.output != "-" {
		var err error
		output, err = os.Create(c.flags.output)
		if err != nil {
			return fmt.Errorf("failed to open output file '%v': %w", c.flags.output, err)
		}
		defer output.Close()
	}

	// Write out
	return result.Write(output, results)
}
