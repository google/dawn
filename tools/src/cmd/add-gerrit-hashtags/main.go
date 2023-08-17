// Copyright 2023 The Tint Authors.
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

// add-gerrit-hashtags adds any missing hashtags parsed from the CL description to the Gerrit change.
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"regexp"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/dawn"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/git"
	"go.chromium.org/luci/auth/client/authcli"
)

const (
	toolName = "add-gerrit-hashtags"
	yyyymmdd = "2006-01-02"
)

var (
	repoFlag    = flag.String("repo", "dawn", "the project (tint or dawn)")
	userFlag    = flag.String("user", defaultUser(), "user name / email")
	afterFlag   = flag.String("after", "", "start date")
	beforeFlag  = flag.String("before", "", "end date")
	daysFlag    = flag.Int("days", 30, "interval in days (used if --after is not specified)")
	verboseFlag = flag.Bool("v", false, "verbose mode - lists all the changes")
	dryrunFlag  = flag.Bool("dry", false, "dry mode. Don't apply any changes")
	authFlags   = authcli.Flags{}
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
	authFlags.Register(flag.CommandLine, auth.DefaultAuthOptions())

	flag.Usage = func() {
		out := flag.CommandLine.Output()
		fmt.Fprintf(out, "%v adds any missing hashtags parsed from the CL description to the Gerrit change.\n", toolName)
		fmt.Fprintf(out, "\n")
		flag.PrintDefaults()
	}
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
		before = time.Now().Add(24 * time.Hour)
	}
	if *afterFlag != "" {
		after, err = time.Parse(yyyymmdd, *afterFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse after date: %w", err)
		}
	} else {
		after = before.Add(-time.Hour * time.Duration(24**daysFlag))
	}

	ctx := context.Background()
	auth, err := authFlags.Options()
	if err != nil {
		return err
	}

	g, err := gerrit.New(ctx, auth, dawn.GerritURL)
	if err != nil {
		return err
	}

	submitted, _, err := g.QueryChanges(
		"owner:"+user,
		"after:"+date(after),
		"before:"+date(before),
		"repo:"+*repoFlag)
	if err != nil {
		return fmt.Errorf("Query failed: %w", err)
	}

	numUpdated := 0
	for _, cl := range submitted {
		expected := parseHashtags(cl.Subject)
		got := container.NewSet(cl.Hashtags...)
		if !got.ContainsAll(expected) {
			toAdd := expected.Clone()
			toAdd.RemoveAll(got)
			fmt.Printf("%v: %v missing hashtags: %v\n", cl.Number, cl.Subject, strings.Join(toAdd.List(), ", "))
			if !*dryrunFlag {
				if err := g.AddHashtags(cl.ChangeID, toAdd); err != nil {
					return err
				}
				numUpdated++
			}
		}
	}

	if numUpdated > 0 {
		fmt.Println()
		fmt.Println(numUpdated, "changes updated with new hashtags")
	} else {
		fmt.Println("no changes updated")
	}

	return nil
}

var reBracketHashtag = regexp.MustCompile(`\[(\w+)\]`)
var reColonHashtag = regexp.MustCompile(`^(\w+):`)

func parseHashtags(subject string) container.Set[string] {
	out := container.NewSet[string]()
	for _, match := range reBracketHashtag.FindAllStringSubmatch(subject, -1) {
		out.Add(match[1])
	}
	if match := reColonHashtag.FindStringSubmatch(subject); len(match) > 1 {
		out.Add(match[1])
	}
	return out
}

func today() time.Time {
	return time.Now()
}

func date(t time.Time) string {
	return t.Format(yyyymmdd)
}
