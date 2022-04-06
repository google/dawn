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

package git

import (
	"context"
	"encoding/hex"
	"errors"
	"fmt"
	"net/url"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

// Hash is a 20 byte, git object hash.
type Hash [20]byte

func (h Hash) String() string { return hex.EncodeToString(h[:]) }

// IsZero returns true if the hash h is all zeros
func (h Hash) IsZero() bool {
	zero := Hash{}
	return h == zero
}

// ParseHash returns a Hash from a hexadecimal string.
func ParseHash(s string) (Hash, error) {
	b, err := hex.DecodeString(s)
	if err != nil {
		return Hash{}, fmt.Errorf("failed to parse hash '%v':\n  %w", s, err)
	}
	h := Hash{}
	copy(h[:], b)
	return h, nil
}

// The timeout for git operations if no other timeout is specified
var DefaultTimeout = time.Minute

// Git wraps the 'git' executable
type Git struct {
	// Path to the git executable
	exe string
	// Debug flag to print all command to the `git` executable
	LogAllActions bool
}

// New returns a new Git instance
func New(exe string) (*Git, error) {
	if _, err := os.Stat(exe); err != nil {
		return nil, err
	}
	return &Git{exe: exe}, nil
}

// Auth holds git authentication credentials
type Auth struct {
	Username string
	Password string
}

// Empty return true if there's no username or password for authentication
func (a Auth) Empty() bool {
	return a.Username == "" && a.Password == ""
}

// ErrRepositoryDoesNotExist indicates that a repository does not exist
var ErrRepositoryDoesNotExist = errors.New("repository does not exist")

// Open opens an existing git repo at path. If the repository does not exist at
// path then ErrRepositoryDoesNotExist is returned.
func (g Git) Open(path string) (*Repository, error) {
	info, err := os.Stat(filepath.Join(path, ".git"))
	if err != nil || !info.IsDir() {
		return nil, ErrRepositoryDoesNotExist
	}
	return &Repository{g, path}, nil
}

// Optional settings for Git.Clone
type CloneOptions struct {
	// If specified then the given branch will be cloned instead of the default
	Branch string
	// Timeout for the operation
	Timeout time.Duration
	// Authentication for the clone
	Auth Auth
}

// Clone performs a clone of the repository at url to path.
func (g Git) Clone(path, url string, opt *CloneOptions) (*Repository, error) {
	if err := os.MkdirAll(path, 0777); err != nil {
		return nil, err
	}
	if opt == nil {
		opt = &CloneOptions{}
	}
	url, err := opt.Auth.addToURL(url)
	if err != nil {
		return nil, err
	}
	r := &Repository{g, path}
	args := []string{"clone", url, "."}
	if opt.Branch != "" {
		args = append(args, "--branch", opt.Branch)
	}
	if _, err := r.run(opt.Timeout, args...); err != nil {
		return nil, err
	}
	return r, nil
}

// Repository points to a git repository
type Repository struct {
	// Path to the 'git' executable
	Git Git
	// Repo directory
	Path string
}

// Optional settings for Repository.Fetch
type FetchOptions struct {
	// The remote name. Defaults to 'origin'
	Remote string
	// Timeout for the operation
	Timeout time.Duration
	// Git authentication for the remote
	Auth Auth
}

// Fetch performs a fetch of a reference from the remote, returning the Hash of
// the fetched reference.
func (r Repository) Fetch(ref string, opt *FetchOptions) (Hash, error) {
	if opt == nil {
		opt = &FetchOptions{}
	}
	if opt.Remote == "" {
		opt.Remote = "origin"
	}
	if _, err := r.run(opt.Timeout, "fetch", opt.Remote, ref); err != nil {
		return Hash{}, err
	}
	out, err := r.run(0, "rev-parse", "FETCH_HEAD")
	if err != nil {
		return Hash{}, err
	}
	return ParseHash(out)
}

// Optional settings for Repository.Push
type PushOptions struct {
	// The remote name. Defaults to 'origin'
	Remote string
	// Timeout for the operation
	Timeout time.Duration
	// Git authentication for the remote
	Auth Auth
}

// Push performs a push of the local reference to the remote reference.
func (r Repository) Push(localRef, remoteRef string, opt *PushOptions) error {
	if opt == nil {
		opt = &PushOptions{}
	}
	if opt.Remote == "" {
		opt.Remote = "origin"
	}
	url, err := r.run(opt.Timeout, "remote", "get-url", opt.Remote)
	if err != nil {
		return err
	}
	url, err = opt.Auth.addToURL(url)
	if err != nil {
		return err
	}
	if _, err := r.run(opt.Timeout, "push", url, localRef+":"+remoteRef); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Add
type AddOptions struct {
	// Timeout for the operation
	Timeout time.Duration
	// Git authentication for the remote
	Auth Auth
}

// Add stages the listed files
func (r Repository) Add(path string, opt *AddOptions) error {
	if opt == nil {
		opt = &AddOptions{}
	}
	if _, err := r.run(opt.Timeout, "add", path); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Commit
type CommitOptions struct {
	// Timeout for the operation
	Timeout time.Duration
	// Author name
	AuthorName string
	// Author email address
	AuthorEmail string
}

// Commit commits the staged files with the given message, returning the hash of
// commit
func (r Repository) Commit(msg string, opt *CommitOptions) (Hash, error) {
	if opt == nil {
		opt = &CommitOptions{}
	}
	args := []string{"commit", "-m", msg}
	if opt.AuthorName != "" || opt.AuthorEmail != "" {
		args = append(args, "--author", fmt.Sprintf("%v <%v>", opt.AuthorName, opt.AuthorEmail))
	}
	if _, err := r.run(opt.Timeout, args...); err != nil {
		return Hash{}, err
	}
	out, err := r.run(0, "rev-parse", "HEAD")
	if err != nil {
		return Hash{}, err
	}
	return ParseHash(out)
}

// Optional settings for Repository.Checkout
type CheckoutOptions struct {
	// Timeout for the operation
	Timeout time.Duration
}

// Checkout performs a checkout of a reference.
func (r Repository) Checkout(ref string, opt *CheckoutOptions) error {
	if opt == nil {
		opt = &CheckoutOptions{}
	}
	if _, err := r.run(opt.Timeout, "checkout", ref); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Log
type LogOptions struct {
	// The git reference to the oldest commit in the range to query.
	From string
	// The git reference to the newest commit in the range to query.
	To string
	// The maximum number of entries to return.
	Count int
	// Timeout for the operation
	Timeout time.Duration
}

// CommitInfo describes a single git commit
type CommitInfo struct {
	Hash        Hash
	Date        time.Time
	Author      string
	Subject     string
	Description string
}

// Log returns the list of commits between two references (inclusive).
// The first returned commit is the most recent.
func (r Repository) Log(opt *LogOptions) ([]CommitInfo, error) {
	if opt == nil {
		opt = &LogOptions{}
	}
	args := []string{"log"}
	rng := "HEAD"
	if opt.To != "" {
		rng = opt.To
	}
	if opt.From != "" {
		rng = opt.From + "^.." + rng
	}
	args = append(args, rng, "--pretty=format:ǁ%Hǀ%cIǀ%an <%ae>ǀ%sǀ%b")
	if opt.Count != 0 {
		args = append(args, fmt.Sprintf("-%d", opt.Count))
	}
	out, err := r.run(opt.Timeout, args...)
	if err != nil {
		return nil, err
	}
	return parseLog(out)
}

func (r Repository) run(timeout time.Duration, args ...string) (string, error) {
	return r.Git.run(r.Path, timeout, args...)
}

func (r Repository) runAll(timeout time.Duration, args ...[]string) error {
	for _, a := range args {
		if _, err := r.run(timeout, a...); err != nil {
			return err
		}
	}
	return nil
}

func (g Git) run(dir string, timeout time.Duration, args ...string) (string, error) {
	if timeout == 0 {
		timeout = DefaultTimeout
	}
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	cmd := exec.CommandContext(ctx, g.exe, args...)
	cmd.Dir = dir
	if g.LogAllActions {
		fmt.Printf("%v> %v %v\n", dir, g.exe, strings.Join(args, " "))
	}
	out, err := cmd.CombinedOutput()
	if g.LogAllActions {
		fmt.Println(string(out))
	}
	if err != nil {
		return string(out), fmt.Errorf("%v> %v %v failed:\n  %w\n%v",
			dir, g.exe, strings.Join(args, " "), err, string(out))
	}
	return strings.TrimSpace(string(out)), nil
}

func (a Auth) addToURL(u string) (string, error) {
	if !a.Empty() {
		modified, err := url.Parse(u)
		if err != nil {
			return "", fmt.Errorf("failed to parse url '%v': %v", u, err)
		}
		modified.User = url.UserPassword(a.Username, a.Password)
		u = modified.String()
	}
	return u, nil
}

func parseLog(str string) ([]CommitInfo, error) {
	msgs := strings.Split(str, "ǁ")
	cls := make([]CommitInfo, 0, len(msgs))
	for _, s := range msgs {
		if parts := strings.Split(s, "ǀ"); len(parts) == 5 {
			hash, err := ParseHash(parts[0])
			if err != nil {
				return nil, err
			}
			date, err := time.Parse(time.RFC3339, parts[1])
			if err != nil {
				return nil, err
			}
			cl := CommitInfo{
				Hash:        hash,
				Date:        date,
				Author:      strings.TrimSpace(parts[2]),
				Subject:     strings.TrimSpace(parts[3]),
				Description: strings.TrimSpace(parts[4]),
			}

			cls = append(cls, cl)
		}
	}
	return cls, nil
}
