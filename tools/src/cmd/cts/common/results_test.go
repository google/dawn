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
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"github.com/stretchr/testify/assert"
)

/*******************************************************************************
 * Fake implementations
 ******************************************************************************/

// A fake version of dawn/tools/src/resultsdb's BigQueryClient.
type mockedBigQueryClient struct {
	returnValues []resultsdb.QueryResult
}

func (bq mockedBigQueryClient) QueryTestResults(
	ctx context.Context, builds []buildbucket.BuildID, testPrefix string, f func(*resultsdb.QueryResult) error) error {
	for _, result := range bq.returnValues {
		if err := f(&result); err != nil {
			return err
		}
	}
	return nil
}

/*******************************************************************************
 * GetRawResults tests
 ******************************************************************************/

func generateGoodGetRawResultsInputs() (context.Context, Config, *mockedBigQueryClient, BuildsByName) {
	ctx := context.Background()

	cfg := Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
	}

	client := &mockedBigQueryClient{
		returnValues: []resultsdb.QueryResult{
			resultsdb.QueryResult{
				TestId:   "prefix_test",
				Status:   "PASS",
				Tags:     []resultsdb.TagPair{},
				Duration: 1.0,
			},
		},
	}

	builds := make(BuildsByName)

	return ctx, cfg, client, builds
}

// Tests that valid results are properly parsed and returned.
func TestGetRawResultsHappyPath(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetRawResultsInputs()
	client.returnValues = []resultsdb.QueryResult{
		resultsdb.QueryResult{
			TestId:   "prefix_test_1",
			Status:   "PASS",
			Tags:     []resultsdb.TagPair{},
			Duration: 1.0,
		},
		resultsdb.QueryResult{
			TestId: "prefix_test_2",
			Status: "FAIL",
			Tags: []resultsdb.TagPair{
				resultsdb.TagPair{
					Key:   "javascript_duration",
					Value: "0.5s",
				},
			},
			Duration: 2.0,
		},
		resultsdb.QueryResult{
			TestId: "prefix_test_3",
			Status: "SKIP",
			Tags: []resultsdb.TagPair{
				resultsdb.TagPair{
					Key:   "may_exonerate",
					Value: "true",
				},
			},
			Duration: 3.0,
		},
		resultsdb.QueryResult{
			TestId: "prefix_test_4",
			Status: "SomeStatus",
			Tags: []resultsdb.TagPair{
				resultsdb.TagPair{
					Key:   "typ_tag",
					Value: "linux",
				},
				resultsdb.TagPair{
					Key:   "typ_tag",
					Value: "intel",
				},
			},
			Duration: 4.0,
		},
	}

	expectedResultsList := result.List{
		result.Result{
			Query:        query.Parse("_test_1"),
			Status:       result.Pass,
			Tags:         result.NewTags(),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_2"),
			Status:       result.Failure,
			Tags:         result.NewTags(),
			Duration:     500 * time.Millisecond,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_3"),
			Status:       result.Skip,
			Tags:         result.NewTags(),
			Duration:     3 * time.Second,
			MayExonerate: true,
		},
		result.Result{
			Query:        query.Parse("_test_4"),
			Status:       result.Unknown,
			Tags:         result.NewTags("linux", "intel"),
			Duration:     4 * time.Second,
			MayExonerate: false,
		},
	}

	expectedResults := make(result.ResultsByExecutionMode)
	expectedResults["execution_mode"] = expectedResultsList

	results, err := GetRawResults(ctx, cfg, client, builds)
	assert.Nil(t, err)
	assert.Equal(t, results, expectedResults)
}

// Tests that a mismatched prefix results in an error.
func TestGetRawResultsPrefixMismatch(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetRawResultsInputs()
	client.returnValues[0].TestId = "bad_test"

	results, err := GetRawResults(ctx, cfg, client, builds)
	assert.Nil(t, results)
	assert.ErrorContains(t, err, "Test ID bad_test did not start with prefix even though query should have filtered.")
}

// Tests that a JavaScript duration that cannot be parsed results in an error.
func TestGetRawResultsBadJavaScriptDuration(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetRawResultsInputs()
	client.returnValues[0].Tags = []resultsdb.TagPair{
		resultsdb.TagPair{
			Key:   "javascript_duration",
			Value: "1000foo",
		},
	}

	results, err := GetRawResults(ctx, cfg, client, builds)
	assert.Nil(t, results)
	assert.ErrorContains(t, err, `time: unknown unit "foo" in duration "1000foo"`)
}

// Tests that a non-boolean may_exonerate value results in an error.
func TestGetRawResultsBadMayExonerate(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetRawResultsInputs()
	client.returnValues[0].Tags = []resultsdb.TagPair{
		resultsdb.TagPair{
			Key:   "may_exonerate",
			Value: "yesnt",
		},
	}

	results, err := GetRawResults(ctx, cfg, client, builds)
	assert.Nil(t, results)
	assert.ErrorContains(t, err, `strconv.ParseBool: parsing "yesnt": invalid syntax`)
}
