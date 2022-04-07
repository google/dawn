// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package glob provides file globbing utilities
package glob

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/match"
)

// Scan walks all files and subdirectories from root, returning those
// that Config.shouldExamine() returns true for.
func Scan(root string, cfg Config) ([]string, error) {
	files := []string{}
	err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
		rel, err := filepath.Rel(root, path)
		if err != nil {
			rel = path
		}

		if rel == ".git" {
			return filepath.SkipDir
		}

		if !cfg.shouldExamine(root, path) {
			return nil
		}

		if !info.IsDir() {
			files = append(files, rel)
		}

		return nil
	})
	if err != nil {
		return nil, err
	}
	return files, nil
}

// Configs is a slice of Config.
type Configs []Config

// Config is used to parse the JSON configuration file.
type Config struct {
	// Paths holds a number of JSON objects that contain either a "includes" or
	// "excludes" key to an array of path patterns.
	// Each path pattern is considered in turn to either include or exclude the
	// file path for license scanning. Pattern use forward-slashes '/' for
	// directory separators, and may use the following wildcards:
	//  ?  - matches any single non-separator character
	//  *  - matches any sequence of non-separator characters
	//  ** - matches any sequence of characters including separators
	//
	// Rules are processed in the order in which they are declared, with later
	// rules taking precedence over earlier rules.
	//
	// All files are excluded before the first rule is evaluated.
	//
	// Example:
	//
	// {
	//   "paths": [
	// 	  { "exclude": [ "out/*", "build/*" ] },
	// 	  { "include": [ "out/foo.txt" ] }
	//   ],
	// }
	Paths searchRules
}

// LoadConfig loads a config file at path.
func LoadConfig(path string) (Config, error) {
	cfgBody, err := ioutil.ReadFile(path)
	if err != nil {
		return Config{}, err
	}
	return ParseConfig(string(cfgBody))
}

// ParseConfig parses the config from a JSON string.
func ParseConfig(config string) (Config, error) {
	d := json.NewDecoder(strings.NewReader(config))
	cfg := Config{}
	if err := d.Decode(&cfg); err != nil {
		return Config{}, err
	}
	return cfg, nil
}

// MustParseConfig parses the config from a JSON string, panicing if the config
// does not parse
func MustParseConfig(config string) Config {
	d := json.NewDecoder(strings.NewReader(config))
	cfg := Config{}
	if err := d.Decode(&cfg); err != nil {
		panic(fmt.Errorf("Failed to parse config: %w\nConfig:\n%v", err, config))
	}
	return cfg
}

// rule is a search path predicate.
// root is the project relative path.
// cond is the value to return if the rule doesn't either include or exclude.
type rule func(path string, cond bool) bool

// searchRules is a ordered list of search rules.
// searchRules is its own type as it has to perform custom JSON unmarshalling.
type searchRules []rule

// UnmarshalJSON unmarshals the array of rules in the form:
// { "include": [ ... ] } or { "exclude": [ ... ] }
func (l *searchRules) UnmarshalJSON(body []byte) error {
	type parsed struct {
		Include []string
		Exclude []string
	}

	p := []parsed{}
	if err := json.NewDecoder(bytes.NewReader(body)).Decode(&p); err != nil {
		return err
	}

	*l = searchRules{}
	for _, rule := range p {
		rule := rule
		switch {
		case len(rule.Include) > 0 && len(rule.Exclude) > 0:
			return fmt.Errorf("Rule cannot contain both include and exclude")
		case len(rule.Include) > 0:
			tests := make([]match.Test, len(rule.Include))
			for i, pattern := range rule.Include {
				test, err := match.New(pattern)
				if err != nil {
					return err
				}
				tests[i] = test
			}
			*l = append(*l, func(path string, cond bool) bool {
				for _, test := range tests {
					if test(path) {
						return true
					}
				}
				return cond
			})
		case len(rule.Exclude) > 0:
			tests := make([]match.Test, len(rule.Exclude))
			for i, pattern := range rule.Exclude {
				test, err := match.New(pattern)
				if err != nil {
					return err
				}
				tests[i] = test
			}
			*l = append(*l, func(path string, cond bool) bool {
				for _, test := range tests {
					if test(path) {
						return false
					}
				}
				return cond
			})
		}
	}
	return nil
}

// shouldExamine returns true if the file at absPath should be scanned.
func (c Config) shouldExamine(root, absPath string) bool {
	root = filepath.ToSlash(root)       // Canonicalize
	absPath = filepath.ToSlash(absPath) // Canonicalize
	relPath, err := filepath.Rel(root, absPath)
	if err != nil {
		return false
	}
	relPath = filepath.ToSlash(relPath) // Canonicalize

	res := false
	for _, rule := range c.Paths {
		res = rule(relPath, res)
	}

	return res
}
