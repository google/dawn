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

// Package resultsdb provides helpers for interfacing with resultsdb
package resultsdb

import (
	"context"
	"fmt"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"go.chromium.org/luci/auth"
	"go.chromium.org/luci/grpc/prpc"
	"go.chromium.org/luci/hardcoded/chromeinfra"
	rdbpb "go.chromium.org/luci/resultdb/proto/v1"
	"google.golang.org/protobuf/types/known/fieldmaskpb"
)

// ResultsDB is the client to communicate with ResultDB.
type ResultsDB struct {
	client rdbpb.ResultDBClient
}

// New creates a client to communicate with ResultDB.
func New(ctx context.Context, credentials auth.Options) (*ResultsDB, error) {
	http, err := auth.NewAuthenticator(ctx, auth.InteractiveLogin, credentials).Client()
	if err != nil {
		return nil, err
	}
	client, err := rdbpb.NewResultDBPRPCClient(
		&prpc.Client{
			C:       http,
			Host:    chromeinfra.ResultDBHost,
			Options: prpc.DefaultOptions(),
		}), nil
	if err != nil {
		return nil, err
	}

	return &ResultsDB{client}, nil
}

// QueryTestResults fetches the test results for the given builds.
// f is called once per page of test variants.
func (r *ResultsDB) QueryTestResults(
	ctx context.Context,
	builds []buildbucket.BuildID,
	filterRegex string,
	f func(*rdbpb.TestResult) error) error {

	invocationNames := make([]string, len(builds))
	for i, id := range builds {
		invocationNames[i] = fmt.Sprintf("invocations/build-%v", id)
	}

	pageToken := ""
	for {
		rsp, err := r.client.QueryTestResults(ctx, &rdbpb.QueryTestResultsRequest{
			Invocations: invocationNames,
			Predicate: &rdbpb.TestResultPredicate{
				TestIdRegexp: filterRegex,
			},
			ReadMask: &fieldmaskpb.FieldMask{Paths: []string{
				"test_id", "status", "tags", "duration",
			}},
			PageSize:  1000, // Maximum page size.
			PageToken: pageToken,
		})
		if err != nil {
			return err
		}

		for _, res := range rsp.TestResults {
			if err := f(res); err != nil {
				return err
			}
		}

		pageToken = rsp.GetNextPageToken()
		if pageToken == "" {
			break
		}
	}

	return nil
}
