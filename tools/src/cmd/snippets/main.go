// Copyright 2021 The Tint Authors.
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

// snippets gathers information about changes merged for weekly reports (snippets).
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/dawn"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/git"
)

const yyyymmdd = "2006-01-02"

var (
	// See https://dawn-review.googlesource.com/new-password for obtaining
	// username and password for gerrit.
	gerritUser = flag.String("gerrit-user", "", "gerrit authentication username")
	gerritPass = flag.String("gerrit-pass", "", "gerrit authentication password")
	userFlag   = flag.String("user", defaultUser(), "user name / email")
	afterFlag  = flag.String("after", "", "start date")
	beforeFlag = flag.String("before", "", "end date")
	daysFlag   = flag.Int("days", 7, "interval in days (used if --after is not specified)")
)

func defaultUser() string {
	if gitExe, err := exec.LookPath("git"); err == nil {
		if g, err := git.New(gitExe); err == nil {
			if cwd, err := os.Getwd(); err == nil {
				if r, err := g.Open(cwd); err == nil {
					if cfg, err := r.Config(nil); err == nil {
						return cfg["user.email"]
					}
				}
			}
		}
	}
	return ""
}

func main() {
	flag.Parse()
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func run() error {
	var after, before time.Time
	var err error
	user := *userFlag
	if user == "" {
		return fmt.Errorf("Missing required 'user' flag")
	}
	if *beforeFlag != "" {
		before, err = time.Parse(yyyymmdd, *beforeFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse before date: %w", err)
		}
	} else {
		before = time.Now()
	}
	if *afterFlag != "" {
		after, err = time.Parse(yyyymmdd, *afterFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse after date: %w", err)
		}
	} else {
		after = before.Add(-time.Hour * time.Duration(24**daysFlag))
	}

	g, err := gerrit.New(dawn.GerritURL, gerrit.Credentials{
		Username: *gerritUser, Password: *gerritPass,
	})
	if err != nil {
		return err
	}

	submitted, _, err := g.QueryChanges(
		"status:merged",
		"owner:"+user,
		"after:"+date(after),
		"before:"+date(before))
	if err != nil {
		return fmt.Errorf("Query failed: %w", err)
	}

	changesByProject := map[string][]string{}
	for _, change := range submitted {
		str := fmt.Sprintf(`* [%s](%sc/%s/+/%d)`, change.Subject, dawn.GerritURL, change.Project, change.Number)
		changesByProject[change.Project] = append(changesByProject[change.Project], str)
	}

	for _, project := range []string{"tint", "dawn"} {
		if changes := changesByProject[project]; len(changes) > 0 {
			fmt.Println("##", strings.Title(project))
			for _, change := range changes {
				fmt.Println(change)
			}
			fmt.Println()
		}
	}

	return nil
}

func today() time.Time {
	return time.Now()
}

func date(t time.Time) string {
	return t.Format(yyyymmdd)
}
