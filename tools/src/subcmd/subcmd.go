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

// Package subcmd provides a multi-command interface for command line tools.
package subcmd

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"net/http"
	"net/http/pprof"
	"os"
	"path/filepath"
	"strings"
	"text/tabwriter"
)

// ErrInvalidCLA is the error returned when an invalid command line argument was
// provided, and the usage was already printed.
var ErrInvalidCLA = errors.New("invalid command line args")

// InvalidCLA shows the flag usage, and returns ErrInvalidCLA
func InvalidCLA() error {
	flag.Usage()
	return ErrInvalidCLA
}

// Command is the interface for a command
// Data is a generic data type passed down to the sub-command when run.
type Command[Data any] interface {
	// Name returns the name of the command.
	Name() string
	// Desc returns a description of the command.
	Desc() string
	// RegisterFlags registers all the command-specific flags
	// Returns a list of mandatory arguments that must immediately follow the
	// command name
	RegisterFlags(context.Context, Data) ([]string, error)
	// Run invokes the command
	Run(context.Context, Data) error
}

// Run handles the parses the command line arguments, possibly invoking one of
// the provided commands.
// If the command line arguments are invalid, then an error message is printed
// and Run returns ErrInvalidCLA.
func Run[Data any](ctx context.Context, data Data, cmds ...Command[Data]) error {
	_, exe := filepath.Split(os.Args[0])

	flag.Usage = func() {
		out := flag.CommandLine.Output()
		tw := tabwriter.NewWriter(out, 0, 1, 0, ' ', 0)
		fmt.Fprintln(tw, exe, "[command]")
		fmt.Fprintln(tw)
		fmt.Fprintln(tw, "Commands:")
		for _, cmd := range cmds {
			fmt.Fprintln(tw, "  ", cmd.Name(), "\t-", cmd.Desc())
		}
		fmt.Fprintln(tw)
		fmt.Fprintln(tw, "Common flags:")
		tw.Flush()
		flag.PrintDefaults()
	}

	profile := false
	flag.BoolVar(&profile, "profile", false, "enable a webserver at localhost:8080/profile that exposes a CPU profiler")
	mux := http.NewServeMux()
	mux.HandleFunc("/profile", pprof.Profile)

	if len(os.Args) < 2 {
		return InvalidCLA()
	}
	help := os.Args[1] == "help"
	if help {
		copy(os.Args[1:], os.Args[2:])
		os.Args = os.Args[:len(os.Args)-1]
	}

	for _, cmd := range cmds {
		if cmd.Name() == os.Args[1] {
			out := flag.CommandLine.Output()
			mandatory, err := cmd.RegisterFlags(ctx, data)
			if err != nil {
				return err
			}
			flag.Usage = func() {
				flagsAndArgs := append([]string{"<flags>"}, mandatory...)
				fmt.Fprintln(out, exe, cmd.Name(), strings.Join(flagsAndArgs, " "))
				fmt.Fprintln(out)
				fmt.Fprintln(out, cmd.Desc())
				fmt.Fprintln(out)
				fmt.Fprintln(out, "flags:")
				flag.PrintDefaults()
			}
			if help {
				flag.Usage()
				return nil
			}
			args := os.Args[2:] // all arguments after the exe and command
			if err := flag.CommandLine.Parse(args); err != nil {
				return err
			}
			if nonFlagArgs := flag.Args(); len(nonFlagArgs) < len(mandatory) {
				fmt.Fprintln(out, "missing argument", mandatory[len(nonFlagArgs)])
				fmt.Fprintln(out)
				return InvalidCLA()
			}
			if profile {
				fmt.Println("download profile at: localhost:8080/profile")
				fmt.Println("then run: 'go tool pprof <file>")
				go http.ListenAndServe(":8080", mux)
			}
			return cmd.Run(ctx, data)
		}
	}

	return InvalidCLA()
}
