// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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

package node

import (
	"errors"
	"fmt"
	"io"
	"os"
	"os/exec"
	"strings"
)

// target starts command-line runner processes.
type target interface {
	// start runs the runner with args, feeding it stdin.
	start(args []string, stdin string) (process, error)
}

// process is a running command-line runner process.
type process interface {
	// output returns the process's stdout and stderr, interleaved.
	output() io.Reader
	// wait waits for the process to exit.
	wait() exitStatus
	// kill stops the process.
	kill()
}

// exitStatus describes how a process ended.
type exitStatus struct {
	success     bool   // it exited with status 0
	description string // e.g. "exit status 1" or "signal: aborted (core dumped)"
}

// localTarget runs the runner on this machine.
type localTarget struct {
	runner         string // path to the executable
	dir            string // working directory, or "" for the current directory
	verbose        bool   // print each command before running it
	skipVSCodeInfo bool   // skip VS Code launch.json output when dir is set
}

func (t *localTarget) start(args []string, stdin string) (process, error) {
	cmd := exec.Command(t.runner, args...)
	cmd.Dir = t.dir
	if t.verbose {
		if t.dir != "" {
			PrintCommand(cmd, t.skipVSCodeInfo)
		} else {
			fmt.Println("Running", strings.Join(cmd.Args, " "))
		}
	}
	return startWithPipe(cmd, stdin)
}

// localProcess is a process started by exec.
type localProcess struct {
	cmd *exec.Cmd
	out *os.File
}

// startWithPipe starts cmd reading stdin, with stdout and stderr both writing
// to one pipe, so that the reader sees them in the order they were written.
func startWithPipe(cmd *exec.Cmd, stdin string) (*localProcess, error) {
	r, w, err := os.Pipe()
	if err != nil {
		return nil, err
	}
	cmd.Stdin = strings.NewReader(stdin)
	cmd.Stdout = w
	cmd.Stderr = w
	err = cmd.Start()
	// The child has its own copy of w. Closing ours lets r reach EOF when the
	// child exits.
	w.Close()
	if err != nil {
		r.Close()
		return nil, err
	}
	return &localProcess{cmd: cmd, out: r}, nil
}

func (p *localProcess) output() io.Reader {
	return p.out
}

// waitErr waits for the process to exit and returns the error from exec.Cmd.Wait.
func (p *localProcess) waitErr() error {
	err := p.cmd.Wait()
	p.out.Close()
	return err
}

func (p *localProcess) wait() exitStatus {
	err := p.waitErr()
	return exitStatus{success: err == nil, description: exitDescription(err)}
}

func (p *localProcess) kill() {
	p.cmd.Process.Kill()
}

// exitDescription describes the error returned by exec.Cmd.Wait, e.g.
// "exit status 1" or "signal: aborted (core dumped)".
func exitDescription(err error) string {
	var exitErr *exec.ExitError
	switch {
	case err == nil:
		return "exit status 0"
	case errors.As(err, &exitErr):
		return exitErr.ProcessState.String()
	default:
		return err.Error()
	}
}
