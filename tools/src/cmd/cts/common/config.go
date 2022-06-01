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
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"github.com/tidwall/jsonc"
)

// Config holds the configuration data for the 'cts' command.
// Config is loaded from a JSON file stored next to the
// tools/src/cmd/cts/config.json.
type Config struct {
	// Test holds configuration data for test results.
	Test struct {
		// The ResultDB string prefix for CTS tests.
		Prefix string
		// The time threshold used to classify tests as slow.
		SlowThreshold time.Duration
	}
	// Gerrit holds configuration for Dawn's Gerrit server.
	Gerrit struct {
		// The host URL
		Host string
		// The project name
		Project string
	}
	// Git holds configuration data for the various Git repositories.
	Git struct {
		// The CTS git repository.
		CTS GitProject
		// The Dawn git repository.
		Dawn GitProject
	}
	// Builders is a map of builder name (as displayed in the UI) to buildbucket
	// builder information.
	Builders map[string]buildbucket.Builder
	// Tags holds configuration data for cleaning result tags before processing
	Tag struct {
		// Remove holds tags that should be removed before processing.
		// See crbug.com/dawn/1401 for more information.
		Remove []string
	}
	// Sheets holds information about the Google Sheets document used for
	// tracking CTS statistics.
	Sheets struct {
		ID string
	}
}

// GitProject holds a git host URL and project.
type GitProject struct {
	Host    string
	Project string
}

// HttpsURL returns the https URL of the project
func (g GitProject) HttpsURL() string {
	return fmt.Sprintf("https://%v/%v", g.Host, g.Project)
}

// LoadConfig loads the JSON config file at the given path
func LoadConfig(path string) (*Config, error) {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to open '%v': %w", path, err)
	}

	// Remove comments, trailing commas.
	data = jsonc.ToJSONInPlace(data)

	cfg := Config{}
	if err := json.NewDecoder(bytes.NewReader(data)).Decode(&cfg); err != nil {
		return nil, fmt.Errorf("failed to load config: %w", err)
	}
	return &cfg, nil
}
