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
	"sort"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/transform"
)

// Directory holds information about a build target
type Target struct {
	// The target's name
	Name TargetName
	// The kind of target
	Kind TargetKind
	// The directory holding the target
	Directory *Directory
	// All project-relative paths of source files that are part of this target
	SourceFileSet container.Set[string]
	// Target names of all dependencies of this target
	DependencyNames container.Set[TargetName]
	// All external dependencies used by this target
	ExternalDependencyMap container.Map[ExternalDependencyName, ExternalDependency]
	// An optional condition for building this target
	Condition string
}

// AddSourceFile adds the File to the target's source set
func (t *Target) AddSourceFile(f *File) {
	t.SourceFileSet.Add(f.Path())
}

// SourceFiles returns the sorted list of the target's source files
func (t *Target) SourceFiles() []*File {
	out := make([]*File, len(t.SourceFileSet))
	for i, name := range t.SourceFileSet.List() {
		out[i] = t.Directory.Project.Files[name]
	}
	return out
}

// SourceFiles returns the sorted list of the target's source files that have no build condition
func (t *Target) UnconditionalSourceFiles() []*File {
	return transform.Filter(t.SourceFiles(), func(t *File) bool { return t.Condition == "" })
}

// AddDependency adds dep to this target's list of dependencies
func (t *Target) AddDependency(dep *Target) {
	if dep != t {
		t.DependencyNames.Add(dep.Name)
	}
}

// AddExternalDependency adds the external dependency with the given name to this target's list of
// external dependencies with the given condition
func (t *Target) AddExternalDependency(name ExternalDependencyName, condition string) {
	if existing, ok := t.ExternalDependencyMap[name]; ok && existing.Condition != condition {
		panic("external dependency added twice with different conditions")
	}
	t.ExternalDependencyMap.Add(name, ExternalDependency{Name: name, Condition: condition})
}

// Dependencies returns the sorted list of dependencies of this target
func (t *Target) Dependencies() []*Target {
	out := make([]*Target, len(t.DependencyNames))
	for i, name := range t.DependencyNames.List() {
		out[i] = t.Directory.Project.Targets[name]
	}
	return out
}

// UnconditionalDependencies returns the sorted list of dependencies that have no build condition.
func (t *Target) UnconditionalDependencies() []*Target {
	return transform.Filter(t.Dependencies(), func(t *Target) bool { return t.Condition == "" })
}

// ExternalDependencies returns the sorted list of external dependencies.
func (t *Target) ExternalDependencies() []ExternalDependency {
	out := make([]ExternalDependency, 0, len(t.ExternalDependencyMap))
	for _, name := range t.ExternalDependencyMap.Keys() {
		out = append(out, t.ExternalDependencyMap[name])
	}
	return out
}

// ConditionalExternalDependencies returns the sorted list of external dependencies that have a
// build condition.
func (t *Target) ConditionalExternalDependencies() []ExternalDependency {
	return transform.Filter(t.ExternalDependencies(), func(t ExternalDependency) bool { return t.Condition != "" })
}

// ConditionalExternalDependencies returns the sorted list of external dependencies that have no
// build condition.
func (t *Target) UnconditionalExternalDependencies() []ExternalDependency {
	return transform.Filter(t.ExternalDependencies(), func(t ExternalDependency) bool { return t.Condition == "" })
}

// A collection of source files and dependencies sharing the same condition
type TargetConditional struct {
	Condition            string
	SourceFiles          []*File
	InternalDependencies []*Target
	ExternalDependencies []ExternalDependency
}

// Conditionals returns a sorted list of TargetConditional, which are grouped by condition
func (t *Target) Conditionals() []*TargetConditional {
	m := container.NewMap[string, *TargetConditional]()
	for name := range t.SourceFileSet {
		file := t.Directory.Project.Files[name]
		if file.Condition != "" {
			c := m.GetOrCreate(file.Condition, func() *TargetConditional {
				return &TargetConditional{Condition: file.Condition}
			})
			c.SourceFiles = append(c.SourceFiles, file)
		}
	}
	for name := range t.DependencyNames {
		dep := t.Directory.Project.Targets[name]
		if dep.Condition != "" {
			c := m.GetOrCreate(dep.Condition, func() *TargetConditional {
				return &TargetConditional{Condition: dep.Condition}
			})
			c.InternalDependencies = append(c.InternalDependencies, dep)
		}
	}
	for _, dep := range t.ExternalDependencyMap {
		if dep.Condition != "" {
			c := m.GetOrCreate(dep.Condition, func() *TargetConditional {
				return &TargetConditional{Condition: dep.Condition}
			})
			c.ExternalDependencies = append(c.ExternalDependencies, dep)
		}
	}
	for _, c := range m {
		sort.Slice(c.SourceFiles, func(a, b int) bool { return c.SourceFiles[a].Name < c.SourceFiles[b].Name })
		sort.Slice(c.InternalDependencies, func(a, b int) bool { return c.InternalDependencies[a].Name < c.InternalDependencies[b].Name })
		sort.Slice(c.ExternalDependencies, func(a, b int) bool { return c.ExternalDependencies[a].Name < c.ExternalDependencies[b].Name })
	}
	return m.Values()
}
