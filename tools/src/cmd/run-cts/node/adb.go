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
	"bufio"
	"errors"
	"fmt"
	"io"
	"os/exec"
	"path"
	"strconv"
	"strings"
	"syscall"
	"time"
)

// Paths on the device, relative to the device directory.
const (
	deviceRunner = "bin/" + runnerName
	deviceCTS    = "webgpu-cts"
)

// adbTarget runs the runner on an Android device through adb.
type adbTarget struct {
	adb       string // path to adb
	serial    string // serial of the device, or "" for adb's default device
	deviceDir string // directory holding the runner and the CTS on the device
	verbose   bool   // print each command before running it
}

// devicePath returns the device path of rel, relative to the device directory.
func (t *adbTarget) devicePath(rel string) string {
	return path.Join(t.deviceDir, rel)
}

// command returns an adb command for the target device.
func (t *adbTarget) command(args ...string) *exec.Cmd {
	if t.serial != "" {
		args = append([]string{"-s", t.serial}, args...)
	}
	return exec.Command(t.adb, args...)
}

// shell runs command in a shell on the device and returns its output.
func (t *adbTarget) shell(command string) (string, error) {
	out, err := t.command("shell", command).CombinedOutput()
	if err != nil {
		return string(out), fmt.Errorf("adb shell '%v' failed: %w\n%s", command, err, out)
	}
	return string(out), nil
}

func (t *adbTarget) start(args []string, stdin string) (process, error) {
	// `echo PID:$$` gets the shell's PID. `exec` will replace the shell with the
	// device runner. The device runner will have the same PID that the shell had.
	words := append([]string{t.devicePath(deviceRunner)}, args...)
	remote := "echo PID:$$; exec " + shellQuoteAll(words) + " 2>&1"
	cmd := t.command("shell", remote)
	if t.verbose {
		fmt.Println("Running", strings.Join(cmd.Args, " "))
	}
	local, err := startWithPipe(cmd, stdin)
	if err != nil {
		return nil, err
	}

	reader := bufio.NewReader(local.out)

	// Store the lines before the PID, so they can be forwarded to the caller.
	preamble := ""
	for {
		line, err := reader.ReadString('\n')
		if pid, ok := parsePIDLine(line); ok {
			// This streams all of the output from the command except for the PID line.
			// All output can be handled by the caller.
			out := io.MultiReader(strings.NewReader(preamble), reader)
			return &adbProcess{local: local, target: t, pid: pid, out: out}, nil
		}
		preamble += line
		if err != nil {
			exit := local.wait()
			return nil, fmt.Errorf("adb shell did not start the runner: %v\n%s", exit.description, preamble)
		}
	}
}

// adbProcess is a runner process on the device, controlled through a local
// adb client.
type adbProcess struct {
	local  *localProcess // the adb client
	target *adbTarget
	pid    int       // PID of the runner on the device
	out    io.Reader // the client's output after the PID line
}

func (p *adbProcess) output() io.Reader {
	return p.out
}

func (p *adbProcess) wait() exitStatus {
	err := p.local.waitErr()
	return exitStatus{success: err == nil, description: adbExitDescription(err)}
}

// kill kills the runner on the device. Killing the local adb client would
// leave the runner running there, so the client is only killed if the runner's
// death does not end it.
func (p *adbProcess) kill() {
	p.target.shell(fmt.Sprintf("kill -9 %d", p.pid))
	time.AfterFunc(10*time.Second, p.local.kill)
}

// parsePIDLine returns the PID printed by "echo PID:$$".
func parsePIDLine(line string) (int, bool) {
	rest, found := strings.CutPrefix(strings.TrimRight(line, "\r\n"), "PID:")
	if !found {
		return 0, false
	}
	pid, err := strconv.Atoi(rest)
	return pid, err == nil
}

// adbExitDescription describes the error returned by waiting for adb shell.
// adb exits with the status of the device process, or with 128 plus the
// signal number if a signal killed it.
func adbExitDescription(err error) string {
	description := exitDescription(err)
	var exitErr *exec.ExitError
	if errors.As(err, &exitErr) {
		if code := exitErr.ExitCode(); code > 128 && code <= 128+64 {
			signal := syscall.Signal(code - 128)
			description += fmt.Sprintf(" (signal %d on the device: %v)", int(signal), signal)
		}
	}
	return description
}

// safeShellChars are the characters that need no quoting in a POSIX shell word.
const safeShellChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789%+,-./:=@_"

// shellQuote quotes s as a single word for a POSIX shell.
func shellQuote(s string) string {
	if s != "" && strings.Trim(s, safeShellChars) == "" {
		return s
	}
	// A single-quoted word cannot contain ', so each ' ends the quoted part, adds
	// an escaped \', and starts a new quoted part: it's becomes 'it'\''s'.
	return "'" + strings.ReplaceAll(s, "'", `'\''`) + "'"
}

// shellQuoteAll quotes each word and joins them with spaces.
func shellQuoteAll(words []string) string {
	quoted := make([]string, len(words))
	for i, word := range words {
		quoted[i] = shellQuote(word)
	}
	return strings.Join(quoted, " ")
}
