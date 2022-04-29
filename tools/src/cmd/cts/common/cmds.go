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

package common

import (
	"dawn.googlesource.com/dawn/tools/src/subcmd"
)

// The registered commands
var commands []Command

// Command is the type of a single cts command
type Command = subcmd.Command[Config]

// Register registers the command for use by the 'cts' tool
func Register(c Command) { commands = append(commands, c) }

// Commands returns all the commands registered
func Commands() []Command { return commands }
