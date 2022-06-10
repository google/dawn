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

// Package gitiles provides helpers for interfacing with gitiles
package gitiles

import (
	"context"
	"fmt"
	"net/http"

	"go.chromium.org/luci/common/api/gitiles"
	gpb "go.chromium.org/luci/common/proto/gitiles"
)

// Gitiles is the client to communicate with Gitiles.
type Gitiles struct {
	client  gpb.GitilesClient
	project string
}

// New creates a client to communicate with Gitiles, for the given host and
// project.
func New(ctx context.Context, host, project string) (*Gitiles, error) {
	client, err := gitiles.NewRESTClient(http.DefaultClient, host, false)
	if err != nil {
		return nil, err
	}
	return &Gitiles{client, project}, nil
}

// Hash returns the git hash of the object with the given 'committish' reference.
func (g *Gitiles) Hash(ctx context.Context, ref string) (string, error) {
	res, err := g.client.Log(ctx, &gpb.LogRequest{
		Project:    g.project,
		Committish: ref,
		PageSize:   1,
	})
	if err != nil {
		return "", err
	}
	log := res.GetLog()
	if len(log) == 0 {
		return "", fmt.Errorf("gitiles returned log was empty")
	}
	return log[0].Id, nil
}

// DownloadFile downloads a single file with the given project-relative path at
// the given reference.
func (g *Gitiles) DownloadFile(ctx context.Context, ref, path string) (string, error) {
	res, err := g.client.DownloadFile(ctx, &gpb.DownloadFileRequest{
		Project:    g.project,
		Committish: ref,
		Path:       path,
	})
	if err != nil {
		return "", err
	}
	return res.GetContents(), nil
}

// ListFiles lists the file paths in a project-relative path at the given reference.
func (g *Gitiles) ListFiles(ctx context.Context, ref, path string) ([]string, error) {
	res, err := g.client.ListFiles(ctx, &gpb.ListFilesRequest{
		Project:    g.project,
		Committish: ref,
		Path:       path,
	})
	if err != nil {
		return []string{}, err
	}
	files := res.GetFiles()
	paths := make([]string, len(files))
	for i, f := range files {
		paths[i] = f.GetPath()
	}
	return paths, nil
}
