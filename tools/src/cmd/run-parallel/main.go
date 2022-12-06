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

// run-parallel is a tool to run an executable with the provided templated
// arguments across all the hardware threads.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"runtime"
	"strings"
	"sync"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
run-parallel is a tool to run an executable with the provided templated
arguments across all the hardware threads.

Usage:
  run-parallel <executable> [arguments...] -- [per-instance-value...]

  executable         - the path to the executable to run.
  arguments          - a list of arguments to pass to the executable.
                       Any occurrance of $ will be substituted with the
                       per-instance-value for the given invocation.
  per-instance-value - a list of values. The executable will be invoked for each
                       value in this list.`)
	os.Exit(1)
}

func run() error {
	onlyPrintFailures := flag.Bool("only-print-failures", false, "Omit output for processes that did not fail")
	flag.Parse()

	args := flag.Args()
	if len(args) < 2 {
		showUsage()
	}
	exe := args[0]
	args = args[1:]

	var perInstanceValues []string
	for i, arg := range args {
		if arg == "--" {
			perInstanceValues = args[i+1:]
			args = args[:i]
			break
		}
	}
	if perInstanceValues == nil {
		showUsage()
	}

	taskIndices := make(chan int, 64)
	type result struct {
		cmd    string
		msg    string
		failed bool
	}
	results := make([]result, len(perInstanceValues))

	numCPU := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numCPU)
	for i := 0; i < numCPU; i++ {
		go func() {
			defer wg.Done()
			for idx := range taskIndices {
				taskArgs := make([]string, len(args))
				for i, arg := range args {
					taskArgs[i] = strings.ReplaceAll(arg, "$", perInstanceValues[idx])
				}
				success, out := invoke(exe, taskArgs)
				if !success || !*onlyPrintFailures {
					results[idx] = result{fmt.Sprint(append([]string{exe}, taskArgs...)), out, !success}
				}
			}
		}()
	}

	for i := range perInstanceValues {
		taskIndices <- i
	}
	close(taskIndices)

	wg.Wait()

	failed := false
	for _, result := range results {
		if result.msg != "" {
			fmt.Printf("'%v' returned %v\n", result.cmd, result.msg)
		}
		failed = failed || result.failed
	}
	if failed {
		os.Exit(1)
	}
	return nil
}

func invoke(exe string, args []string) (ok bool, output string) {
	cmd := exec.Command(exe, args...)
	out, err := cmd.CombinedOutput()
	str := string(out)
	if err != nil {
		if str != "" {
			return false, str
		}
		return false, err.Error()
	}
	return true, str
}
