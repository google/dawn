// Copyright 2024 The Dawn & Tint Authors
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

package common

import (
	"context"
	"fmt"
	"log"
	"os/exec"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"go.chromium.org/luci/auth"
	"google.golang.org/api/option"
	"google.golang.org/api/sheets/v4"
)

// Export appends the results binned by status and number of unimplemented tests to a Google Sheets document.
func Export(ctx context.Context,
	credentials auth.Options,
	sheetsID, ctsDir, node, npmPath string,
	resultsByExecutionMode result.ResultsByExecutionMode) error {
	// Create the sheets service client
	http, err := auth.NewAuthenticator(ctx, auth.InteractiveLogin, credentials).Client()
	if err != nil {
		return err
	}

	s, err := sheets.NewService(ctx, option.WithHTTPClient(http))
	if err != nil {
		return fmt.Errorf("unable to create sheets client: %w", err)
	}

	// Get the CTS statistics spreadsheet
	spreadsheet, err := s.Spreadsheets.Get(sheetsID).Do()
	if err != nil {
		return fmt.Errorf("failed to get spreadsheet: %w", err)
	}

	// Grab the CTS revision to count the number of unimplemented tests
	numUnimplemented, err := CountUnimplementedTests(ctx, ctsDir, node, npmPath)
	if err != nil {
		return fmt.Errorf("failed to obtain number of unimplemented tests: %w", err)
	}

	// Generate a new set of counts of test by status
	for mode, results := range resultsByExecutionMode {
		name := string(mode) + "-data"
		// Scan the sheets of the spreadsheet (tabs at the bottom) for the sheet named `name`.
		var dataSheet *sheets.Sheet
		for _, sheet := range spreadsheet.Sheets {
			if strings.ToLower(sheet.Properties.Title) == name {
				dataSheet = sheet
				break
			}
		}
		if dataSheet == nil {
			return fmt.Errorf("failed to find sheet '%v'", name)
		}

		// Fetch the table column names
		columns, err := fetchRow[string](s, spreadsheet, dataSheet, 0)

		counts := container.NewMap[string, int]()
		for _, r := range results {
			counts[string(r.Status)] = counts[string(r.Status)] + 1
		}

		{ // Report statuses that have no column
			missingColumns := container.NewSet(counts.Keys()...)
			missingColumns.RemoveAll(container.NewSet(columns...))
			for _, status := range missingColumns.List() {
				log.Println("no column for status", status)
			}
		}

		// Generate new cell data based on the table column names
		data := []any{}
		for _, column := range columns {
			switch strings.ToLower(column) {
			case "date":
				data = append(data, time.Now().UTC().Format("2006-01-02"))
			case "unimplemented":
				data = append(data, numUnimplemented)
			default:
				count, ok := counts[column]
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
			return fmt.Errorf("failed to update spreadsheet: %v\n\ndata: %+v", err, data)
		}
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

// CountUnimplementedTests returns the number of unimplemented tests in the CTS.
// It does this by running the cmdline.ts CTS command with '--list-unimplemented webgpu:*'.
func CountUnimplementedTests(ctx context.Context, ctsDir, node, npmPath string) (int, error) {
	if err := InstallCTSDeps(ctx, ctsDir, npmPath); err != nil {
		return 0, err
	}

	// Run 'src/common/runtime/cmdline.ts' to obtain the full test list
	cmd := exec.CommandContext(ctx, node,
		"-e", "require('./src/common/tools/setup-ts-in-node.js');require('./src/common/runtime/cmdline.ts');",
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		"--list-unimplemented",
		"webgpu:*",
	)
	cmd.Dir = ctsDir

	out, err := cmd.CombinedOutput()
	if err != nil {
		return 0, fmt.Errorf("failed to gather unimplemented tests: %w", err)
	}

	lines := strings.Split(string(out), "\n")
	count := 0
	for _, line := range lines {
		if strings.TrimSpace(line) != "" {
			count++
		}
	}
	return count, nil
}
