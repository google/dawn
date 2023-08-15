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

// Package build implements a extensible build file list and module dependency
// generator for Tint.
// See: docs/tint/gen.md
package build

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"regexp"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/match"
	"dawn.googlesource.com/dawn/tools/src/template"
	"dawn.googlesource.com/dawn/tools/src/transform"
	"github.com/mzohreva/gographviz/graphviz"
	"github.com/tidwall/jsonc"
)

const srcTint = "src/tint"

func init() {
	common.Register(&Cmd{})
}

type Cmd struct {
	flags struct {
		dot bool
	}
}

func (Cmd) Name() string {
	return "build"
}

func (Cmd) Desc() string {
	return `build generates BUILD.* files in each of Tint's source directories`
}

func (c *Cmd) RegisterFlags(ctx context.Context, cfg *common.Config) ([]string, error) {
	flag.BoolVar(&c.flags.dot, "dot", false, "emit GraphViz DOT files for each target kind")
	return nil, nil
}

func (c Cmd) Run(ctx context.Context, cfg *common.Config) error {
	p := NewProject(CanonicalizePath(path.Join(fileutils.DawnRoot(), srcTint)), cfg)

	for _, stage := range []struct {
		desc string
		fn   func(p *Project) error
	}{
		{"loading 'externals.json'", loadExternals},
		{"populating source files", populateSourceFiles},
		{"scanning source files", scanSourceFiles},
		{"loading directory configs", applyDirectoryConfigs},
		{"checking for cycles", checkForCycles},
		{"emitting build files", emitBuildFiles},
	} {
		if cfg.Flags.Verbose {
			log.Printf("%v...\n", stage.desc)
		}
		if err := stage.fn(p); err != nil {
			return err
		}
	}

	if c.flags.dot {
		for _, kind := range AllTargetKinds {
			if err := emitDotFile(p, kind); err != nil {
				return err
			}
		}
	}

	if cfg.Flags.Verbose {
		log.Println("done")
	}
	return nil
}

// loadExternals loads the 'externals.json' file in this directory.
func loadExternals(p *Project) error {
	content, err := os.ReadFile(path.Join(fileutils.ThisDir(), "externals.json"))
	if err != nil {
		return err
	}

	externals := container.NewMap[string, struct {
		IncludePattern string `json:"include_pattern"`
		Condition      string
	}]()
	if err := json.Unmarshal(jsonc.ToJSON(content), &externals); err != nil {
		return fmt.Errorf("failed to parse 'externals.json': %w", err)
	}

	for _, name := range externals.Keys() {
		external := externals[name]
		match, err := match.New(external.IncludePattern)
		if err != nil {
			return fmt.Errorf("'externals.json' matcher error: %w", err)
		}
		p.externals = append(p.externals, projectExternalDependency{
			name:                ExternalDependencyName(name),
			includePatternMatch: match,
			condition:           external.Condition,
		})
	}

	return nil
}

// Globs all the source files, and creates populates the Project with Directory, Target and File.
// File name patterns are used to bin each file into a target for the directory.
func populateSourceFiles(p *Project) error {
	paths, err := glob.Scan(p.Root, glob.MustParseConfig(`{
		"paths": [
			{
				"include": [
					"*/**.cc",
					"*/**.h",
					"*/**.inl"
				]
			},
			{
				"exclude": [
					"fuzzers/**"
				]
			}]
	}`))
	if err != nil {
		return err
	}

	for _, filepath := range paths {
		filepath = CanonicalizePath(filepath)
		dir, name := path.Split(filepath)
		if kind := targetKindFromFilename(name); kind != targetInvalid {
			directory := p.AddDirectory(dir)
			p.AddTarget(directory, kind).AddSourceFile(p.AddFile(filepath))
		}
	}

	return nil
}

// scanSourceFiles scans all the source files for:
// * #includes to build a dependencies between targets
// * 'GEN_BUILD:' directives
func scanSourceFiles(p *Project) error {
	// Include describes a #include in a source file
	type Include struct {
		path string
		line int
	}

	// ParsedFile describes all the includes and conditions found in a source file
	type ParsedFile struct {
		conditions []string
		includes   []Include
	}

	// parseFile parses the source file at 'path' represented by 'file'
	// As this is run concurrently, it must not modify any shared state (including file)
	parseFile := func(path string, file *File) (string, *ParsedFile, error) {
		body, err := os.ReadFile(file.AbsPath())
		if err != nil {
			return path, nil, err
		}
		out := &ParsedFile{}
		for i, line := range strings.Split(string(body), "\n") {
			if match := reCondition.FindStringSubmatch(line); len(match) > 0 {
				out.conditions = append(out.conditions, match[1])
			}
			if !reIgnoreInclude.MatchString(line) {
				if match := reInclude.FindStringSubmatch(line); len(match) > 0 {
					out.includes = append(out.includes, Include{match[1], i + 1})
				}
			}
		}
		return path, out, nil
	}

	// Create a new map by calling parseFile() on each entry of p.Files
	// This is performed over multiple concurrent goroutines.
	parsedFiles, err := transform.GoMap(p.Files, parseFile)
	if err != nil {
		return err
	}

	// For each file, of each target, of each directory...
	for _, dir := range p.Directories {
		for _, target := range dir.Targets() {
			for _, file := range target.SourceFiles() {
				// Retrieve the parsed file information
				parsed := parsedFiles[file.Path()]

				// Apply any conditions
				for _, condition := range parsed.conditions {
					if file.Condition == "" {
						file.Condition = condition
					} else {
						file.Condition += " && " + condition
					}
				}

				// Resolve includes, and add dependencies to the targets
				for _, include := range parsed.includes {
					if strings.HasPrefix(include.path, srcTint) {
						// #include "src/tint/..."
						include.path = include.path[len(srcTint)+1:] // Strip 'src/tint/'

						includeFile := p.File(include.path)
						if includeFile == nil {
							return fmt.Errorf(`%v:%v includes non-existent file '%v'`, file.Path(), include.line, include.path)
						}

						dependencyKind := targetKindFromFilename(include.path)
						if dependencyKind == targetInvalid {
							return fmt.Errorf(`%v:%v unknown target type for include '%v'`, file.Path(), include.line, includeFile.Path())
						}
						if target.Kind == targetLib && dependencyKind == targetTest {
							return fmt.Errorf(`%v:%v lib target must not include test code '%v'`, file.Path(), include.line, includeFile.Path())
						}

						if dependency := p.Target(includeFile.Directory, dependencyKind); dependency != nil {
							target.AddDependency(dependency)
						}
					} else {
						// Check for external includes
						for _, external := range p.externals {
							if external.includePatternMatch(include.path) {
								target.AddExternalDependency(external.name, external.condition)
							}
						}
					}
				}
			}
		}
	}
	return nil
}

// applyDirectoryConfigs loads a 'BUILD.cfg' file in each source directory (if found), and
// applies the config to the Directory and/or Targets.
func applyDirectoryConfigs(p *Project) error {
	// Config for a single target of a directory
	type TargetConfig struct {
		AdditionalDependencies []string
	}
	// Config for a directory
	type DirectoryConfig struct {
		// Condition for all targets in the directory
		Condition string
		// Configuration for the 'lib' target
		Lib TargetConfig
		// Configuration for the 'test' target
		Test TargetConfig
		// Configuration for the 'bench' target
		Bench TargetConfig
		// Configuration for the 'cmd' target
		Cmd TargetConfig
	}

	// For each directory in the project...
	for _, dir := range p.Directories.Values() {
		path := path.Join(dir.AbsPath(), "BUILD.cfg")
		content, err := os.ReadFile(path)
		if err != nil {
			continue
		}

		// Parse the config
		cfg := DirectoryConfig{}
		if err := json.Unmarshal(jsonc.ToJSON(content), &cfg); err != nil {
			return fmt.Errorf("error while parsing '%v': %w", path, err)
		}

		// Apply any directory-level condition
		for _, target := range dir.Targets() {
			target.Condition = cfg.Condition
		}

		// For each target config...
		for _, tc := range []struct {
			cfg  *TargetConfig
			kind TargetKind
		}{
			{&cfg.Lib, targetLib},
			{&cfg.Test, targetTest},
			{&cfg.Bench, targetBench},
			{&cfg.Cmd, targetCmd},
		} {
			// Add any additional dependencies
			for _, depPattern := range tc.cfg.AdditionalDependencies {
				match, err := match.New(depPattern)
				if err != nil {
					return fmt.Errorf("%v: failed to parse %v.AdditionalDependencies '%v': %w", path, tc.kind, depPattern, err)
				}
				additionalDeps := []*Target{}
				for _, target := range p.Targets.Keys() {
					if match(string(target)) {
						additionalDeps = append(additionalDeps, p.Targets[target])
					}
				}
				if len(additionalDeps) == 0 {
					return fmt.Errorf("%v: %v.AdditionalDependencies '%v' did not match any targets", path, tc.kind, depPattern)
				}
				target := p.AddTarget(dir, tc.kind)
				for _, dep := range additionalDeps {
					target.AddDependency(dep)
				}
			}
		}
	}

	return nil
}

// checkForCycles ensures that the graph of target dependencies are acyclic (a DAG)
func checkForCycles(p *Project) error {
	type state int
	const (
		unvisited state = iota
		visiting
		checked
	)

	cache := container.NewMap[TargetName, state]()

	var walk func(t *Target, path []TargetName) error
	walk = func(t *Target, path []TargetName) error {
		switch cache[t.Name] {
		case unvisited:
			cache[t.Name] = visiting
			for _, dep := range t.Dependencies() {
				if err := walk(dep, append(path, dep.Name)); err != nil {
					return err
				}
			}
			cache[t.Name] = checked
		case visiting:
			err := strings.Builder{}
			fmt.Fprintln(&err, "cyclic target dependency found:")
			for _, t := range path {
				fmt.Fprintln(&err, "  ", string(t))
			}
			fmt.Fprintln(&err, "  ", string(t.Name))
			return fmt.Errorf(err.String())
		}
		return nil
	}

	for _, target := range p.Targets.Values() {
		if err := walk(target, []TargetName{target.Name}); err != nil {
			return err
		}
	}
	return nil
}

// emitBuildFiles emits a 'BUILD.*' file in each source directory for each
// 'BUILD.*.tmpl' found in this directory.
func emitBuildFiles(p *Project) error {
	// Glob all the template files
	templatePaths, err := glob.Glob(path.Join(fileutils.ThisDir(), "*.tmpl"))
	if err != nil {
		return err
	}
	if len(templatePaths) == 0 {
		fmt.Println("warning: no BUILD template files found")
		return nil
	}

	// Load the templates
	templates := container.NewMap[string, *template.Template]()
	for _, path := range templatePaths {
		tmpl, err := template.FromFile(path)
		if err != nil {
			return err
		}
		templates[path] = tmpl
	}

	// process executes all the templates for the directory dir
	// This is run concurrently, so must not modify shared state
	process := func(dir *Directory) (common.StaleFiles, error) {
		stale := common.StaleFiles{}

		// For each template...
		for _, tmplPath := range templatePaths {
			_, tmplName := filepath.Split(tmplPath)
			outputName := strings.TrimSuffix(tmplName, ".tmpl")
			outputPath := path.Join(dir.AbsPath(), outputName)

			// Attempt to read the existing output file
			existing, err := os.ReadFile(outputPath)
			if err != nil {
				existing = nil
			}
			// If the file is annotated with a GEN_BUILD:DO_NOT_GENERATE directive, leave it alone
			if reDoNotGenerate.Match(existing) {
				continue
			}
			// Buffer for output
			w := &bytes.Buffer{}

			// Write the header
			relTmplPath, err := filepath.Rel(fileutils.DawnRoot(), tmplPath)
			if err != nil {
				return nil, err
			}
			w.WriteString(common.Header(string(existing), CanonicalizePath(relTmplPath), "#"))

			// Write the template output
			err = templates[tmplPath].Run(w, dir, map[string]any{})
			if err != nil {
				return nil, err
			}

			// Format the output if it's a GN file.
			if path.Ext(outputName) == ".gn" {
				unformatted := w.String()
				gn := exec.Command("gn", "format", "--stdin")
				gn.Stdin = bytes.NewReader([]byte(unformatted))
				w.Reset()
				gn.Stdout = w
				gn.Stderr = w
				if err := gn.Run(); err != nil {
					return nil, fmt.Errorf("%v\ngn format failed: %w\n%v", unformatted, err, w.String())
				}
			}

			if string(existing) != w.String() {
				stale = append(stale, outputPath)
			}

			if !p.cfg.Flags.CheckStale {
				if err := os.WriteFile(outputPath, w.Bytes(), 0666); err != nil {
					return nil, err
				}
			}

		}

		return stale, nil
	}

	// Concurrently run process() on all the directories.
	staleLists, err := transform.GoSlice(p.Directories.Values(), process)
	if err != nil {
		return err
	}

	if p.cfg.Flags.Verbose || p.cfg.Flags.CheckStale {
		// Collect all stale files into a flat list
		stale := transform.Flatten(staleLists)
		if p.cfg.Flags.CheckStale && len(stale) > 0 {
			return stale
		}
		if p.cfg.Flags.Verbose {
			log.Printf("generated %v files\n", len(stale))
		}
	}

	return nil
}

// targetKindFromFilename returns the target kind my pattern matching the filename
func targetKindFromFilename(filename string) TargetKind {
	switch {
	case strings.HasSuffix(filename, "_test.cc"), strings.HasSuffix(filename, "_test.h"):
		return targetTest
	case strings.HasSuffix(filename, "_bench.cc"), strings.HasSuffix(filename, "_bench.h"):
		return targetBench
	case filename == "main.cc":
		return targetCmd
	case strings.HasSuffix(filename, ".cc"), strings.HasSuffix(filename, ".h"):
		return targetLib
	}
	return targetInvalid
}

// emitDotFile writes a GraphViz DOT file visualizing the target dependency graph
func emitDotFile(p *Project, kind TargetKind) error {
	g := graphviz.Graph{}
	nodes := container.NewMap[TargetName, int]()
	targets := []*Target{}
	for _, target := range p.Targets.Values() {
		if target.Kind == kind {
			targets = append(targets, target)
		}
	}
	for _, target := range targets {
		nodes.Add(target.Name, g.AddNode(string(target.Name)))
	}
	for _, target := range targets {
		for _, dep := range target.DependencyNames.List() {
			g.AddEdge(nodes[target.Name], nodes[dep], "")
		}
	}

	g.MakeDirected()

	g.DefaultNodeAttribute(graphviz.Shape, graphviz.ShapeBox)
	g.DefaultNodeAttribute(graphviz.FontName, "Courier")
	g.DefaultNodeAttribute(graphviz.FontSize, "14")
	g.DefaultNodeAttribute(graphviz.Style, graphviz.StyleFilled+","+graphviz.StyleRounded)
	g.DefaultNodeAttribute(graphviz.FillColor, "yellow")

	g.DefaultEdgeAttribute(graphviz.FontName, "Courier")
	g.DefaultEdgeAttribute(graphviz.FontSize, "12")

	file, err := os.Create(path.Join(p.Root, fmt.Sprintf("%v.dot", kind)))
	if err != nil {
		return err
	}
	defer file.Close()

	g.GenerateDOT(file)
	return nil
}

var (
	// Regular expressions used by this file
	reInclude       = regexp.MustCompile(`\s*#\s*include\s*\"([^"]+)\"`)
	reIgnoreInclude = regexp.MustCompile(`//\s*GEN_BUILD:IGNORE`)
	reCondition     = regexp.MustCompile(`//\s*GEN_BUILD:CONDITION\((.*)\)\s*$`)
	reDoNotGenerate = regexp.MustCompile(`#\s*GEN_BUILD:DO_NOT_GENERATE`)
)
