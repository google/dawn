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

package build

import (
	"path"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cnf"
	"dawn.googlesource.com/dawn/tools/src/container"
)

// Directory holds information about a directory holding source files
type Directory struct {
	// The project for this directory
	Project *Project
	// The parent directory
	Parent *Directory
	// The name of the directory
	Name string
	// The project-relative path of the directory
	Path string
	// The names of all subdirectories of this directory
	SubdirectoryNames container.Set[string]
	// The names of all targets of this directory
	TargetNames container.Set[TargetName]
}

// AbsPath returns an absolute path for this directory
func (d *Directory) AbsPath() string {
	return path.Join(d.Project.Root, d.Path)
}

// Depth returns the number of nested directories this directory is from the
// project root.
func (d *Directory) Depth() int {
	return strings.Count(d.Path, "/")
}

// Targets returns a sorted list of targets of this directory
func (d *Directory) Targets() []*Target {
	out := make([]*Target, len(d.TargetNames))
	for i, name := range d.TargetNames.List() {
		out[i] = d.Project.Targets[name]
	}
	return out
}

// Subdirectories returns a sorted list of subdirectories of this directory
func (d *Directory) Subdirectories() []*Directory {
	out := make([]*Directory, len(d.SubdirectoryNames))
	for i, name := range d.SubdirectoryNames.List() {
		out[i] = d.Project.Directories[path.Join(d.Path, name)]
	}
	return out
}

// DecomposedConditionals returns the combined decomposed ANDs, ORs and unary expressions of all
// the conditional expressions used by the targets in this directory. This can be used by templates
// that need to break expressions down into separate sub-expressions.
func (d *Directory) DecomposedConditionals() cnf.Decomposed {
	// Gather up all the conditional expressions used by targets in this directory
	expressions := container.NewMap[cnf.Key, cnf.Expr]()
	addExpr := func(expr cnf.Expr) {
		if len(expr) > 0 {
			expressions.Add(expr.Key(), expr)
		}
	}
	for _, target := range d.Targets() {
		addExpr(target.Condition)
		for _, c := range target.Conditionals() {
			addExpr(c.Condition)
		}
	}

	// Build maps for the AND, OR and unary sub-expressions.
	allAnds := container.NewMap[cnf.Key, cnf.Ands]()
	allOrs := container.NewMap[cnf.Key, cnf.Ors]()
	allUnarys := container.NewMap[cnf.Key, cnf.Unary]()

	// Populate the maps
	for _, expr := range expressions {
		decomposed := cnf.Decompose(expr)
		for _, ands := range decomposed.Ands {
			allAnds.Add(ands.Key(), ands)
		}
		for _, ors := range decomposed.Ors {
			allOrs.Add(ors.Key(), ors)
		}
		for _, unarys := range decomposed.Unarys {
			allUnarys.Add(unarys.Key(), unarys)
		}
	}

	// Return the decomposed expressions
	return cnf.Decomposed{
		Ands:   allAnds.Values(),
		Ors:    allOrs.Values(),
		Unarys: allUnarys.Values(),
	}
}
