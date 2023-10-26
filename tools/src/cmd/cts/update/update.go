// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
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

package update

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"go.chromium.org/luci/auth/client/authcli"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		results      common.ResultSource
		expectations string
		auth         authcli.Flags
	}
}

func (cmd) Name() string {
	return "update"
}

func (cmd) Desc() string {
	return "updates a CTS expectations file"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	defaultExpectations := common.DefaultExpectationsPath()
	c.flags.results.RegisterFlags(cfg)
	c.flags.auth.Register(flag.CommandLine, auth.DefaultAuthOptions( /* needsCloudScopes */ false))
	flag.StringVar(&c.flags.expectations, "expectations", defaultExpectations, "path to CTS expectations file to update")
	return nil, nil
}

func loadTestList(path string) ([]query.Query, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to load test list: %w", err)
	}
	lines := strings.Split(string(data), "\n")
	out := make([]query.Query, len(lines))
	for i, l := range lines {
		out[i] = query.Parse(l)
	}
	return out, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Validate command line arguments
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	// Fetch the results
	log.Println("fetching results...")
	results, err := c.flags.results.GetResults(ctx, cfg, auth)
	if err != nil {
		return err
	}

	// Merge to remove duplicates
	log.Println("removing duplicate results...")
	results = result.Merge(results)

	// Load the expectations file
	log.Println("loading expectations...")
	ex, err := expectations.Load(c.flags.expectations)
	if err != nil {
		return err
	}

	log.Println("loading test list...")
	testlist, err := loadTestList(common.DefaultTestListPath())
	if err != nil {
		return err
	}

	log.Println("validating...")
	if diag := ex.Validate(); diag.NumErrors() > 0 {
		diag.Print(os.Stdout, c.flags.expectations)
		return fmt.Errorf("validation failed")
	}

	// Update the expectations file with the results
	log.Println("updating expectations...")
	diag, err := ex.Update(results, testlist)
	if err != nil {
		return err
	}

	// Print any diagnostics
	diag.Print(os.Stdout, c.flags.expectations)

	// Save the updated expectations file
	return ex.Save(c.flags.expectations)
}
