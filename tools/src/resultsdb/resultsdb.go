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
