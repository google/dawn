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
	"fmt"
	"log"
	"sort"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
)

// BuildsByName is a map of builder name to build result
type BuildsByName map[string]buildbucket.Build

func (b BuildsByName) ids() []buildbucket.BuildID {
	ids := make([]buildbucket.BuildID, 0, len(b))
	for _, build := range b {
		ids = append(ids, build.ID)
	}
	return ids
}

// GetBuilds returns the builds, as declared in the config file, for the given
// patchset
func GetBuilds(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	bb *buildbucket.Buildbucket) (BuildsByName, error) {

	builds := BuildsByName{}

	err := bb.SearchBuilds(ctx, ps, func(build buildbucket.Build) error {
		for name, builder := range cfg.Builders {
			if build.Builder == builder {
				builds[name] = build
				break
			}
		}
		return nil
	})
	if err != nil {
		return nil, err
	}

	return builds, err
}

// WaitForBuildsToComplete waits until all the provided builds have finished.
func WaitForBuildsToComplete(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	bb *buildbucket.Buildbucket,
	builds BuildsByName) error {

	buildsStillRunning := func() []string {
		out := []string{}
		for name, build := range builds {
			if build.Status.Running() {
				out = append(out, name)
			}
		}
		sort.Strings(out)
		return out
	}

	for {
		// Refresh build status
		for name, build := range builds {
			build, err := bb.QueryBuild(ctx, build.ID)
			if err != nil {
				return fmt.Errorf("failed to query build for '%v': %w", name, err)
			}
			builds[name] = build
		}
		running := buildsStillRunning()
		if len(running) == 0 {
			break
		}
		log.Println("waiting for builds to complete: ", running)
		time.Sleep(time.Minute * 2)
	}

	for name, build := range builds {
		if build.Status == buildbucket.StatusInfraFailure ||
			build.Status == buildbucket.StatusCanceled {
			return fmt.Errorf("%v builder failed with %v", name, build.Status)
		}
	}

	return nil
}

// GetOrStartBuildsAndWait starts the builds as declared in the config file,
// for the given patchset, if they haven't already been started or if retest is
// true. GetOrStartBuildsAndWait then waits for the builds to complete, and then
// returns the results.
func GetOrStartBuildsAndWait(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	bb *buildbucket.Buildbucket,
	retest bool) (BuildsByName, error) {

	builds := BuildsByName{}

	if !retest {
		// Find any existing builds for the patchset
		err := bb.SearchBuilds(ctx, ps, func(build buildbucket.Build) error {
			for name, builder := range cfg.Builders {
				if build.Builder == builder {
					builds[name] = build
					break
				}
			}
			return nil
		})
		if err != nil {
			return nil, err
		}
	}

	// Returns true if the build should be re-kicked
	shouldKick := func(build buildbucket.Build) bool {
		switch build.Status {
		case buildbucket.StatusUnknown,
			buildbucket.StatusInfraFailure,
			buildbucket.StatusCanceled:
			return true
		}
		return false
	}

	// Kick any missing builds
	for name, builder := range cfg.Builders {
		if build, found := builds[name]; !found || shouldKick(build) {
			build, err := bb.StartBuild(ctx, ps, builder, retest)
			if err != nil {
				return nil, err
			}
			log.Printf("started build: %+v", build)
			builds[name] = build
		}
	}

	if err := WaitForBuildsToComplete(ctx, cfg, ps, bb, builds); err != nil {
		return nil, err
	}

	return builds, nil
}
