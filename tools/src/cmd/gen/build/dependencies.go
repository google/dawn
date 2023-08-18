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
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/transform"
)

// Dependencies describes the dependencies of a target
type Dependencies struct {
	// The project
	project *Project
	// Target names of all dependencies of this target
	internal container.Set[TargetName]
	// All external dependencies used by this target
	external container.Set[ExternalDependencyName]
}

// NewDependencies returns a new Dependencies
func NewDependencies(p *Project) *Dependencies {
	return &Dependencies{
		project:  p,
		internal: container.NewSet[TargetName](),
		external: container.NewSet[ExternalDependencyName](),
	}
}

// AddInternal adds dep to the list of internal dependencies
func (d *Dependencies) AddInternal(dep *Target) {
	d.internal.Add(dep.Name)
}

// AddExternal adds dep to the list of external dependencies
func (d *Dependencies) AddExternal(dep ExternalDependency) {
	d.external.Add(dep.Name)
}

// Internal returns the sorted list of dependencies of this target
func (d *Dependencies) Internal() []*Target {
	out := make([]*Target, len(d.internal))
	for i, name := range d.internal.List() {
		out[i] = d.project.Targets[name]
	}
	return out
}

// UnconditionalInternal returns the sorted list of dependencies that have no build condition.
func (d *Dependencies) UnconditionalInternal() []*Target {
	return transform.Filter(d.Internal(), func(d *Target) bool { return d.Condition == nil })
}

// External returns the sorted list of external dependencies.
func (d *Dependencies) External() []ExternalDependency {
	out := make([]ExternalDependency, 0, len(d.external))
	for _, name := range d.external.List() {
		out = append(out, d.project.externals[name])
	}
	return out
}

// ConditionalExternalDependencies returns the sorted list of external dependencies that have a
// build condition.
func (d *Dependencies) ConditionalExternal() []ExternalDependency {
	return transform.Filter(d.External(), func(e ExternalDependency) bool { return e.Condition != nil })
}

// ConditionalExternalDependencies returns the sorted list of external dependencies that have no
// build condition.
func (d *Dependencies) UnconditionalExternal() []ExternalDependency {
	return transform.Filter(d.External(), func(e ExternalDependency) bool { return e.Condition == nil })
}

// ContainsExternal returns true if the external dependencies contains name
func (d *Dependencies) ContainsExternal(name ExternalDependencyName) bool {
	return d.external.Contains(name)
}
