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
	"os"
	"os/exec"
)

// exitStatus describes how a process ended.
type exitStatus struct {
	success     bool   // it exited with status 0
	description string // e.g. "exit status 1" or "signal: aborted (core dumped)"
}

// localProcess is a process started by exec.
type localProcess struct {
	cmd *exec.Cmd
	out *os.File
}

// startWithPipe starts cmd with stdout and stderr both writing to one pipe, so
// that the reader sees them in the order they were written.
func startWithPipe(cmd *exec.Cmd) (*localProcess, error) {
	r, w, err := os.Pipe()
	if err != nil {
		return nil, err
	}
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

func (p *localProcess) wait() exitStatus {
	err := p.cmd.Wait()
	p.out.Close()
	return exitStatus{success: err == nil, description: exitDescription(err)}
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
