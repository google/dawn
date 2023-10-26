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

package export

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/gitiles"
	"go.chromium.org/luci/auth/client/authcli"
	"golang.org/x/oauth2"
	"golang.org/x/oauth2/google"
	"google.golang.org/api/sheets/v4"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		auth    authcli.Flags
		results common.ResultSource
	}
}

func (cmd) Name() string {
	return "export"
}

func (cmd) Desc() string {
	return "exports the latest CTS results to Google sheets"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.auth.Register(flag.CommandLine, auth.DefaultAuthOptions( /* needsCloudScopes */ false))
	c.flags.results.RegisterFlags(cfg)
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Validate command line arguments
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	// Load the credentials used for accessing the sheets document
	authdir := fileutils.ExpandHome(os.ExpandEnv(auth.SecretsDir))
	credentialsPath := filepath.Join(authdir, "credentials.json")
	b, err := ioutil.ReadFile(credentialsPath)
	if err != nil {
		return fmt.Errorf("unable to read credentials file '%v'\n"+
			"Obtain this file from: https://console.developers.google.com/apis/credentials\n%w",
			credentialsPath, err)
	}
	credentials, err := google.CredentialsFromJSON(ctx, b, "https://www.googleapis.com/auth/spreadsheets")
	if err != nil {
		return fmt.Errorf("unable to parse client secret file to config: %w", err)
	}

	// Create the sheets service client
	s, err := sheets.New(oauth2.NewClient(ctx, credentials.TokenSource))
	if err != nil {
		return fmt.Errorf("unable to create sheets client: %w", err)
	}

	// Get the CTS statistics spreadsheet
	spreadsheet, err := s.Spreadsheets.Get(cfg.Sheets.ID).Do()
	if err != nil {
		return fmt.Errorf("failed to get spreadsheet: %w", err)
	}

	// Scan the sheets of the spreadsheet (tabs at the bottom) for the 'data'
	// sheet.
	var dataSheet *sheets.Sheet
	for _, sheet := range spreadsheet.Sheets {
		if strings.ToLower(sheet.Properties.Title) == "data" {
			dataSheet = sheet
			break
		}
	}
	if dataSheet == nil {
		return fmt.Errorf("failed to find data sheet")
	}

	// Fetch the table column names
	columns, err := fetchRow[string](s, spreadsheet, dataSheet, 0)

	// Grab the results
	results, err := c.flags.results.GetResults(ctx, cfg, auth)
	if err != nil {
		return err
	}
	if len(results) == 0 {
		return fmt.Errorf("no results found")
	}
	ps := c.flags.results.Patchset

	// Find the CTS revision
	dawn, err := gitiles.New(ctx, cfg.Git.Dawn.Host, cfg.Git.Dawn.Project)
	if err != nil {
		return fmt.Errorf("failed to open dawn host: %w", err)
	}
	deps, err := dawn.DownloadFile(ctx, ps.RefsChanges(), "DEPS")
	if err != nil {
		return fmt.Errorf("failed to download DEPS from %v: %w", ps.RefsChanges(), err)
	}
	_, ctsHash, err := common.UpdateCTSHashInDeps(deps, "<unused>")
	if err != nil {
		return fmt.Errorf("failed to find CTS hash in deps: %w", err)
	}

	// Grab the CTS revision to count the number of unimplemented tests
	numUnimplemented, err := countUnimplementedTests(cfg, ctsHash)
	if err != nil {
		return fmt.Errorf("failed to obtain number of unimplemented tests: %w", err)
	}

	// Generate a new set of counts of test by status
	log.Printf("exporting results from cl %v ps %v...", ps.Change, ps.Patchset)
	counts := map[result.Status]int{}
	for _, r := range results {
		counts[r.Status] = counts[r.Status] + 1
	}

	// Generate new cell data based on the table column names
	data := []any{}
	for _, column := range columns {
		switch strings.ToLower(column) {
		case "date":
			data = append(data, time.Now().UTC().Format("2006-01-02"))
		case "change":
			data = append(data, ps.Change)
		case "unimplemented":
			data = append(data, numUnimplemented)
		default:
			count, ok := counts[result.Status(column)]
			if !ok {
				log.Println("no results with status", column)
			}
			data = append(data, count)
		}
	}

	// Insert a blank row under the column header row
	if err := insertBlankRows(s, spreadsheet, dataSheet, 1, 1); err != nil {
		return err
	}

	// Add a new row to the spreadsheet
	_, err = s.Spreadsheets.Values.BatchUpdate(spreadsheet.SpreadsheetId,
		&sheets.BatchUpdateValuesRequest{
			ValueInputOption: "RAW",
			Data: []*sheets.ValueRange{{
				Range:  rowRange(1, dataSheet),
				Values: [][]any{data},
			}},
		}).Do()
	if err != nil {
		return fmt.Errorf("failed to update spreadsheet: %v", err)
	}

	return nil
}

// rowRange returns a sheets range ("name!Ai:i") for the entire row with the
// given index.
func rowRange(index int, sheet *sheets.Sheet) string {
	return fmt.Sprintf("%v!A%v:%v", sheet.Properties.Title, index+1, index+1)
}

// columnRange returns a sheets range ("name!i1:i") for the entire column with
// the given index.
func columnRange(index int, sheet *sheets.Sheet) string {
	col := 'A' + index
	if index > 25 {
		panic("UNIMPLEMENTED")
	}
	return fmt.Sprintf("%v!%c1:%c", sheet.Properties.Title, col, col)
}

// fetchRow returns all the values in the given sheet's row.
func fetchRow[T any](srv *sheets.Service, spreadsheet *sheets.Spreadsheet, sheet *sheets.Sheet, row int) ([]T, error) {
	rng := rowRange(row, sheet)
	data, err := srv.Spreadsheets.Values.Get(spreadsheet.SpreadsheetId, rng).Do()
	if err != nil {
		return nil, fmt.Errorf("Couldn't fetch %v: %w", rng, err)
	}
	out := make([]T, len(data.Values[0]))
	for column, v := range data.Values[0] {
		val, ok := v.(T)
		if !ok {
			return nil, fmt.Errorf("cell at %v:%v was type %T, but expected type %T", row, column, v, val)
		}
		out[column] = val
	}
	return out, nil
}

// insertBlankRows inserts blank rows into the given sheet.
func insertBlankRows(srv *sheets.Service, spreadsheet *sheets.Spreadsheet, sheet *sheets.Sheet, aboveRow, count int) error {
	req := sheets.BatchUpdateSpreadsheetRequest{
		Requests: []*sheets.Request{{
			InsertRange: &sheets.InsertRangeRequest{
				Range: &sheets.GridRange{
					SheetId:       sheet.Properties.SheetId,
					StartRowIndex: int64(aboveRow),
					EndRowIndex:   int64(aboveRow + count),
				},
				ShiftDimension: "ROWS",
			}},
		},
	}
	if _, err := srv.Spreadsheets.BatchUpdate(spreadsheet.SpreadsheetId, &req).Do(); err != nil {
		return fmt.Errorf("BatchUpdate failed: %v", err)
	}
	return nil
}

// countUnimplementedTests checks out the WebGPU CTS at ctsHash, builds the node
// command line tool, and runs it with '--list-unimplemented webgpu:*' to count
// the total number of unimplemented tests, which is returned.
func countUnimplementedTests(cfg common.Config, ctsHash string) (int, error) {
	tmpDir, err := os.MkdirTemp("", "dawn-cts-export")
	if err != nil {
		return 0, err
	}
	defer os.RemoveAll(tmpDir)

	dir := filepath.Join(tmpDir, "cts")

	gitExe, err := exec.LookPath("git")
	if err != nil {
		return 0, fmt.Errorf("failed to find git on PATH: %w", err)
	}

	git, err := git.New(gitExe)
	if err != nil {
		return 0, err
	}

	log.Printf("cloning cts to '%v'...", dir)
	repo, err := git.Clone(dir, cfg.Git.CTS.HttpsURL(), nil)
	if err != nil {
		return 0, fmt.Errorf("failed to clone cts: %v", err)
	}

	log.Printf("checking out cts @ '%v'...", ctsHash)
	if _, err := repo.Fetch(ctsHash, nil); err != nil {
		return 0, fmt.Errorf("failed to fetch cts: %v", err)
	}
	if err := repo.Checkout(ctsHash, nil); err != nil {
		return 0, fmt.Errorf("failed to clone cts: %v", err)
	}

	{
		npm, err := exec.LookPath("npm")
		if err != nil {
			return 0, fmt.Errorf("failed to find npm on PATH: %w", err)
		}
		cmd := exec.Command(npm, "ci")
		cmd.Dir = dir
		if out, err := cmd.CombinedOutput(); err != nil {
			return 0, fmt.Errorf("failed to run npm ci: %w\n%v", err, string(out))
		}
	}
	{
		npx, err := exec.LookPath("npx")
		if err != nil {
			return 0, fmt.Errorf("failed to find npx on PATH: %w", err)
		}
		cmd := exec.Command(npx, "grunt", "run:build-out-node")
		cmd.Dir = dir
		if out, err := cmd.CombinedOutput(); err != nil {
			return 0, fmt.Errorf("failed to build CTS typescript: %w\n%v", err, string(out))
		}
	}
	{
		sh, err := exec.LookPath("node")
		if err != nil {
			return 0, fmt.Errorf("failed to find sh on PATH: %w", err)
		}
		cmd := exec.Command(sh, "./tools/run_node", "--list-unimplemented", "webgpu:*")
		cmd.Dir = dir
		out, err := cmd.CombinedOutput()
		if err != nil {
			return 0, fmt.Errorf("failed to gather unimplemented tests: %w", err)
		}
		lines := strings.Split(string(out), "\n")
		return len(lines), nil
	}
}
