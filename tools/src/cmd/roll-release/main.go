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

// roll-release is a tool to roll changes in Tint release branches into Dawn,
// and create new Tint release branches.
//
// See showUsage() for more information
package main

import (
	"encoding/hex"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"

	"dawn.googlesource.com/tint/tools/src/gerrit"
	"github.com/go-git/go-git/v5"
	"github.com/go-git/go-git/v5/config"
	"github.com/go-git/go-git/v5/plumbing"
	"github.com/go-git/go-git/v5/plumbing/transport"
	git_http "github.com/go-git/go-git/v5/plumbing/transport/http"
	"github.com/go-git/go-git/v5/storage/memory"
)

const (
	toolName            = "roll-release"
	gitCommitMsgHookURL = "https://gerrit-review.googlesource.com/tools/hooks/commit-msg"
	tintURL             = "https://dawn.googlesource.com/tint"
	dawnURL             = "https://dawn.googlesource.com/dawn"
	tintSubdirInDawn    = "third_party/tint"
	branchPrefix        = "chromium/"
	branchLegacyCutoff  = 4590 // Branch numbers < than this are ignored
)

type branches = map[string]plumbing.Hash

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Printf(`
%[1]v is a tool to synchronize Dawn's release branches with Tint.

%[1]v will scan the release branches of both Dawn and Tint, and will:
* Create new Gerrit changes to roll new release branch changes from Tint into
  Dawn.
* Find and create missing Tint release branches, using the git hash of Tint in
  the DEPS file of the Dawn release branch.

%[1]v does not depend on the current state of the Tint checkout, nor will it
make any changes to the local checkout.

usage:
  %[1]v
`, toolName)
	flag.PrintDefaults()
	fmt.Println(``)
	os.Exit(1)
}

func run() error {
	dry := false
	flag.BoolVar(&dry, "dry", false, "perform a dry run")
	flag.Usage = showUsage
	flag.Parse()

	// This tool uses a mix of 'go-git' and the command line git.
	// go-git has the benefit of keeping the git information entirely in-memory,
	// but has issues working with chromiums tools and gerrit.
	// To create new release branches in Tint, we use 'go-git', so we need to
	// dig out the username and password.
	var auth transport.AuthMethod
	if user, pass := gerrit.LoadCredentials(); user != "" {
		auth = &git_http.BasicAuth{Username: user, Password: pass}
	} else {
		return fmt.Errorf("failed to fetch git credentials")
	}

	// Using in-memory repos, find all the tint and dawn release branches
	log.Println("Inspecting dawn and tint release branches...")
	var tint, dawn *git.Repository
	var tintBranches, dawnBranches branches
	for _, r := range []struct {
		name     string
		url      string
		repo     **git.Repository
		branches *branches
	}{
		{"tint", tintURL, &tint, &tintBranches},
		{"dawn", dawnURL, &dawn, &dawnBranches},
	} {
		repo, err := git.Init(memory.NewStorage(), nil)
		if err != nil {
			return fmt.Errorf("failed to create %v in-memory repo: %w", r.name, err)
		}
		remote, err := repo.CreateRemote(&config.RemoteConfig{
			Name: "origin",
			URLs: []string{r.url},
		})
		if err != nil {
			return fmt.Errorf("failed to add %v remote: %w", r.name, err)
		}
		refs, err := remote.List(&git.ListOptions{})
		if err != nil {
			return fmt.Errorf("failed to fetch %v branches: %w", r.name, err)
		}
		branches := branches{}
		for _, ref := range refs {
			if !ref.Name().IsBranch() {
				continue
			}
			name := ref.Name().Short()
			if strings.HasPrefix(name, branchPrefix) {
				branches[name] = ref.Hash()
			}
		}
		*r.repo = repo
		*r.branches = branches
	}

	// Find the release branches found in dawn, which are missing in tint.
	// Find the release branches in dawn that are behind HEAD of the
	// corresponding branch in tint.
	log.Println("Scanning dawn DEPS...")
	type roll struct {
		from, to plumbing.Hash
	}
	tintBranchesToCreate := branches{}      // branch name -> tint hash
	dawnBranchesToRoll := map[string]roll{} // branch name -> roll
	for name := range dawnBranches {
		if isBranchBefore(name, branchLegacyCutoff) {
			continue // Branch is earlier than we're interested in
		}
		deps, err := getDEPS(dawn, name)
		if err != nil {
			return err
		}
		depsTintHash, err := parseTintFromDEPS(deps)
		if err != nil {
			return err
		}

		if tintBranchHash, found := tintBranches[name]; found {
			if tintBranchHash != depsTintHash {
				dawnBranchesToRoll[name] = roll{from: depsTintHash, to: tintBranchHash}
			}
		} else {
			tintBranchesToCreate[name] = depsTintHash
		}
	}

	if dry {
		tasks := []string{}
		for name, sha := range tintBranchesToCreate {
			tasks = append(tasks, fmt.Sprintf("Create Tint release branch '%v' @ %v", name, sha))
		}
		for name, roll := range dawnBranchesToRoll {
			tasks = append(tasks, fmt.Sprintf("Roll Dawn release branch '%v' from %v to %v", name, roll.from, roll.to))
		}
		sort.Strings(tasks)
		fmt.Printf("%v was run with --dry. Run without --dry to:\n", toolName)
		for _, task := range tasks {
			fmt.Println(" >", task)
		}
		return nil
	}

	didSomething := false
	if n := len(tintBranchesToCreate); n > 0 {
		log.Println("Creating", n, "release branches in tint...")

		// In order to create the branches, we need to know what the DEPS
		// hashes are referring to. Perform an in-memory fetch of tint's main
		// branch.
		if _, err := fetch(tint, "main"); err != nil {
			return err
		}

		for name, sha := range tintBranchesToCreate {
			log.Println("Creating branch", name, "@", sha, "...")

			// Pushing a branch by SHA does not work, so we need to create a
			// local branch first. See https://github.com/go-git/go-git/issues/105
			src := plumbing.NewHashReference(plumbing.NewBranchReferenceName(name), sha)
			if err := tint.Storer.SetReference(src); err != nil {
				return fmt.Errorf("failed to create temporary branch: %w", err)
			}

			dst := plumbing.NewBranchReferenceName(name)
			refspec := config.RefSpec(src.Name() + ":" + dst)
			err := tint.Push(&git.PushOptions{
				RefSpecs: []config.RefSpec{refspec},
				Progress: os.Stdout,
				Auth:     auth,
			})
			if err != nil && err != git.NoErrAlreadyUpToDate {
				return fmt.Errorf("failed to push branch: %w", err)
			}
		}
		didSomething = true
	}

	if n := len(dawnBranchesToRoll); n > 0 {
		log.Println("Rolling", n, "release branches in dawn...")

		// Fetch the change-id hook script
		commitMsgHookResp, err := http.Get(gitCommitMsgHookURL)
		if err != nil {
			return fmt.Errorf("failed to fetch the git commit message hook from '%v': %w", gitCommitMsgHookURL, err)
		}
		commitMsgHook, err := ioutil.ReadAll(commitMsgHookResp.Body)
		if err != nil {
			return fmt.Errorf("failed to fetch the git commit message hook from '%v': %w", gitCommitMsgHookURL, err)
		}

		for name, roll := range dawnBranchesToRoll {
			log.Println("Rolling branch", name, "from tint", roll.from, "to", roll.to, "...")
			dir, err := ioutil.TempDir("", "dawn-roll")
			if err != nil {
				return err
			}
			defer os.RemoveAll(dir)

			// Clone dawn into dir
			if err := call(dir, "git", "clone", "--depth", "1", "-b", name, dawnURL, "."); err != nil {
				return fmt.Errorf("failed to clone dawn branch %v: %w", name, err)
			}

			// Copy the Change-Id hook into the dawn directory
			gitHooksDir := filepath.Join(dir, ".git", "hooks")
			if err := os.MkdirAll(gitHooksDir, 0777); err != nil {
				return fmt.Errorf("failed create commit hooks directory: %w", err)
			}
			if err := ioutil.WriteFile(filepath.Join(gitHooksDir, "commit-msg"), commitMsgHook, 0777); err != nil {
				return fmt.Errorf("failed install commit message hook: %w", err)
			}

			// Clone tint into third_party directory of dawn
			tintDir := filepath.Join(dir, tintSubdirInDawn)
			if err := os.MkdirAll(tintDir, 0777); err != nil {
				return fmt.Errorf("failed to create directory %v: %w", tintDir, err)
			}
			if err := call(tintDir, "git", "clone", "-b", name, tintURL, "."); err != nil {
				return fmt.Errorf("failed to clone tint hash %v: %w", roll.from, err)
			}

			// Checkout tint at roll.from
			if err := call(tintDir, "git", "checkout", roll.from); err != nil {
				return fmt.Errorf("failed to checkout tint at %v: %w", roll.from, err)
			}

			// Use roll-dep to roll tint to roll.to
			if err := call(dir, "roll-dep", "--ignore-dirty-tree", fmt.Sprintf("--roll-to=%s", roll.to), tintSubdirInDawn); err != nil {
				return err
			}

			// Push the change to gerrit
			if err := call(dir, "git", "push", "origin", "HEAD:refs/for/"+name); err != nil {
				return fmt.Errorf("failed to push roll to gerrit: %w", err)
			}
		}
		didSomething = true
	}

	if !didSomething {
		log.Println("Everything up to date")
	} else {
		log.Println("Done")
	}
	return nil
}

// returns true if the branch name contains a branch number less than 'version'
func isBranchBefore(name string, version int) bool {
	n, err := strconv.Atoi(strings.TrimPrefix(name, branchPrefix))
	if err != nil {
		return false
	}
	return n < version
}

// call invokes the executable 'exe' with the given arguments in the working
// directory 'dir'.
func call(dir, exe string, args ...interface{}) error {
	s := make([]string, len(args))
	for i, a := range args {
		s[i] = fmt.Sprint(a)
	}
	cmd := exec.Command(exe, s...)
	cmd.Dir = dir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("%v returned %v", cmd, err)
	}
	return nil
}

// getDEPS returns the content of the DEPS file for the given branch.
func getDEPS(r *git.Repository, branch string) (string, error) {
	hash, err := fetch(r, branch)
	if err != nil {
		return "", err
	}
	commit, err := r.CommitObject(hash)
	if err != nil {
		return "", fmt.Errorf("failed to fetch commit: %w", err)
	}
	tree, err := commit.Tree()
	if err != nil {
		return "", fmt.Errorf("failed to fetch tree: %w", err)
	}
	deps, err := tree.File("DEPS")
	if err != nil {
		return "", fmt.Errorf("failed to find DEPS: %w", err)
	}
	return deps.Contents()
}

// fetch performs a git-fetch of the given branch into 'r', returning the
// fetched branch's hash.
func fetch(r *git.Repository, branch string) (plumbing.Hash, error) {
	src := plumbing.NewBranchReferenceName(branch)
	dst := plumbing.NewRemoteReferenceName("origin", branch)
	err := r.Fetch(&git.FetchOptions{
		RefSpecs: []config.RefSpec{config.RefSpec("+" + src + ":" + dst)},
	})
	if err != nil {
		return plumbing.Hash{}, fmt.Errorf("failed to fetch branch %v: %w", branch, err)
	}
	ref, err := r.Reference(plumbing.ReferenceName(dst), true)
	if err != nil {
		return plumbing.Hash{}, fmt.Errorf("failed to resolve branch %v: %w", branch, err)
	}
	return ref.Hash(), nil
}

var reDEPSTintVersion = regexp.MustCompile("tint@([0-9a-fA-F]*)")

// parseTintFromDEPS returns the tint hash from the DEPS file content 'deps'
func parseTintFromDEPS(deps string) (plumbing.Hash, error) {
	m := reDEPSTintVersion.FindStringSubmatch(deps)
	if len(m) != 2 {
		return plumbing.Hash{}, fmt.Errorf("failed to find tint hash in DEPS")
	}
	b, err := hex.DecodeString(m[1])
	if err != nil {
		return plumbing.Hash{}, fmt.Errorf("failed to find parse tint hash in DEPS: %w", err)
	}
	var h plumbing.Hash
	copy(h[:], b)
	return h, nil
}
