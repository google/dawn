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

import "path"

// Include describes a single #include in a file
type Include struct {
	Path      string
	Line      int
	Condition Condition
}

// File holds information about a source file
type File struct {
	// The directory that holds this source file
	Directory *Directory
	// The target that this source file belongs to
	Target *Target
	// The name of the file
	Name string
	// An optional condition used to build this source file
	Condition Condition
	// All the #include made by this file
	Includes []Include
	// All the transitive dependencies of this file
	TransitiveDependencies *Dependencies
}

// Path returns the project-relative path of the file
func (f *File) Path() string {
	return path.Join(f.Directory.Path, f.Name)
}

// AbsPath returns the absolute path of the file
func (f *File) AbsPath() string {
	return path.Join(f.Directory.AbsPath(), f.Name)
}

// RemoveFromProject removes the File from the project
func (f *File) RemoveFromProject() {
	path := f.Path()
	f.Target.SourceFileSet.Remove(path)
	f.Directory.Project.Files.Remove(path)
}
