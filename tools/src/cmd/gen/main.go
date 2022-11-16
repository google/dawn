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

// gen parses the <tint>/src/tint/intrinsics.def file, then scans the
// project directory for '<file>.tmpl' files, to produce source code files.
package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
	"regexp"
	"runtime"
	"strconv"
	"strings"
	"text/template"
	"time"
	"unicode"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/gen"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/parser"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/resolver"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

const defProjectRelPath = "src/tint/intrinsics.def"

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
gen generates the templated code for the Tint compiler

gen accepts a list of file paths to the templates to generate. If no templates
are explicitly specified, then gen scans the '<dawn>/src/tint' and
'<dawn>/test/tint' directories for '<file>.tmpl' files.

If the templates use the 'IntrinsicTable' function then gen will parse and
resolve the <tint>/src/tint/intrinsics.def file.

usage:
  gen [flags] [template files]

optional flags:`)
	flag.PrintDefaults()
	fmt.Println(``)
	os.Exit(1)
}

func run() error {
	outputDir := ""
	verbose := false
	checkStale := false
	flag.StringVar(&outputDir, "o", "", "custom output directory (optional)")
	flag.BoolVar(&verbose, "verbose", false, "print verbose output")
	flag.BoolVar(&checkStale, "check-stale", false, "don't emit anything, just check that files are up to date")
	flag.Parse()

	staleFiles := []string{}
	projectRoot := fileutils.DawnRoot()

	// Find clang-format
	clangFormatPath := findClangFormat(projectRoot)
	if clangFormatPath == "" {
		return fmt.Errorf("cannot find clang-format in <dawn>/buildtools nor PATH")
	}

	files := flag.Args()
	if len(files) == 0 {
		// Recursively find all the template files in the <dawn>/src/tint and
		// <dawn>/test/tint and directories
		var err error
		files, err = glob.Scan(projectRoot, glob.MustParseConfig(`{
			"paths": [{"include": [
				"src/tint/**.tmpl",
				"test/tint/**.tmpl"
			]}]
		}`))
		if err != nil {
			return err
		}
	} else {
		// Make all template file paths project-relative
		for i, f := range files {
			abs, err := filepath.Abs(f)
			if err != nil {
				return fmt.Errorf("failed to get absolute file path for '%v': %w", f, err)
			}
			if !strings.HasPrefix(abs, projectRoot) {
				return fmt.Errorf("template '%v' is not under project root '%v'", abs, projectRoot)
			}
			rel, err := filepath.Rel(projectRoot, abs)
			if err != nil {
				return fmt.Errorf("failed to get project relative file path for '%v': %w", f, err)
			}
			files[i] = rel
		}
	}

	cache := &genCache{}

	// For each template file...
	for _, relTmplPath := range files { // relative to project root
		if verbose {
			fmt.Println("processing", relTmplPath)
		}
		// Make tmplPath absolute
		tmplPath := filepath.Join(projectRoot, relTmplPath)
		tmplDir := filepath.Dir(tmplPath)

		// Read the template file
		tmpl, err := ioutil.ReadFile(tmplPath)
		if err != nil {
			return fmt.Errorf("failed to open '%v': %w", tmplPath, err)
		}

		// Create or update the file at relPath if the file content has changed,
		// preserving the copyright year in the header.
		// relPath is a path relative to the template
		writeFile := func(relPath, body string) error {
			var outPath string
			if outputDir != "" {
				relTmplDir := filepath.Dir(relTmplPath)
				outPath = filepath.Join(outputDir, relTmplDir, relPath)
			} else {
				outPath = filepath.Join(tmplDir, relPath)
			}

			copyrightYear := time.Now().Year()

			// Load the old file
			existing, err := ioutil.ReadFile(outPath)
			if err == nil {
				// Look for the existing copyright year
				if match := copyrightRegex.FindStringSubmatch(string(existing)); len(match) == 2 {
					if year, err := strconv.Atoi(match[1]); err == nil {
						copyrightYear = year
					}
				}
			}

			// Write the common file header
			if verbose {
				fmt.Println("  writing", outPath)
			}
			sb := strings.Builder{}
			sb.WriteString(fmt.Sprintf(header, copyrightYear, filepath.ToSlash(relTmplPath)))
			sb.WriteString(body)
			oldContent, newContent := string(existing), sb.String()

			if oldContent != newContent {
				if checkStale {
					staleFiles = append(staleFiles, outPath)
				} else {
					if err := os.MkdirAll(filepath.Dir(outPath), 0777); err != nil {
						return fmt.Errorf("failed to create directory for '%v': %w", outPath, err)
					}
					if err := ioutil.WriteFile(outPath, []byte(newContent), 0666); err != nil {
						return fmt.Errorf("failed to write file '%v': %w", outPath, err)
					}
				}
			}

			return nil
		}

		// Write the content generated using the template and semantic info
		sb := strings.Builder{}
		if err := generate(string(tmpl), cache, &sb, writeFile); err != nil {
			return fmt.Errorf("while processing '%v': %w", tmplPath, err)
		}

		if body := sb.String(); body != "" {
			_, tmplFileName := filepath.Split(tmplPath)
			outFileName := strings.TrimSuffix(tmplFileName, ".tmpl")

			switch filepath.Ext(outFileName) {
			case ".cc", ".h", ".inl":
				body, err = clangFormat(body, clangFormatPath)
				if err != nil {
					return err
				}
			}

			if err := writeFile(outFileName, body); err != nil {
				return err
			}
		}
	}

	if len(staleFiles) > 0 {
		fmt.Println(len(staleFiles), "files need regenerating:")
		for _, path := range staleFiles {
			if rel, err := filepath.Rel(projectRoot, path); err == nil {
				fmt.Println(" •", rel)
			} else {
				fmt.Println(" •", path)
			}
		}
		fmt.Println("Regenerate these files with: ./tools/run gen")
		os.Exit(1)
	}

	return nil
}

// Cache for objects that are expensive to build, and can be reused between templates.
type genCache struct {
	cached struct {
		sem            *sem.Sem            // lazily built by sem()
		intrinsicTable *gen.IntrinsicTable // lazily built by intrinsicTable()
		permuter       *gen.Permuter       // lazily built by permute()
	}
}

// sem lazily parses and resolves the intrinsic.def file, returning the semantic info.
func (g *genCache) sem() (*sem.Sem, error) {
	if g.cached.sem == nil {
		// Load the builtins definition file
		defPath := filepath.Join(fileutils.DawnRoot(), defProjectRelPath)

		defSource, err := ioutil.ReadFile(defPath)
		if err != nil {
			return nil, err
		}

		// Parse the definition file to produce an AST
		ast, err := parser.Parse(string(defSource), defProjectRelPath)
		if err != nil {
			return nil, err
		}

		// Resolve the AST to produce the semantic info
		sem, err := resolver.Resolve(ast)
		if err != nil {
			return nil, err
		}

		g.cached.sem = sem
	}
	return g.cached.sem, nil
}

// intrinsicTable lazily calls and returns the result of buildIntrinsicTable(),
// caching the result for repeated calls.
func (g *genCache) intrinsicTable() (*gen.IntrinsicTable, error) {
	if g.cached.intrinsicTable == nil {
		sem, err := g.sem()
		if err != nil {
			return nil, err
		}
		g.cached.intrinsicTable, err = gen.BuildIntrinsicTable(sem)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.intrinsicTable, nil
}

// permute lazily calls buildPermuter(), caching the result for repeated
// calls, then passes the argument to Permutator.Permute()
func (g *genCache) permute(overload *sem.Overload) ([]gen.Permutation, error) {
	if g.cached.permuter == nil {
		sem, err := g.sem()
		if err != nil {
			return nil, err
		}
		g.cached.permuter, err = gen.NewPermuter(sem)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.permuter.Permute(overload)
}

var copyrightRegex = regexp.MustCompile(`// Copyright (\d+) The`)

const header = `// Copyright %v The Tint Authors.
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

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   %v
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

`

type generator struct {
	template  *template.Template
	cache     *genCache
	writeFile WriteFile
	rnd       *rand.Rand
}

// WriteFile is a function that Generate() may call to emit a new file from a
// template.
// relPath is the relative path from the currently executing template.
// content is the file content to write.
type WriteFile func(relPath, content string) error

// generate executes the template tmpl, writing the output to w.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func generate(tmpl string, cache *genCache, w io.Writer, writeFile WriteFile) error {
	g := generator{
		template:  template.New("<template>"),
		cache:     cache,
		writeFile: writeFile,
		rnd:       rand.New(rand.NewSource(4561123)),
	}
	if err := g.bindAndParse(g.template, tmpl); err != nil {
		return err
	}
	return g.template.Execute(w, nil)
}

func (g *generator) bindAndParse(t *template.Template, text string) error {
	_, err := t.Funcs(map[string]interface{}{
		"Map":                   newMap,
		"Iterate":               iterate,
		"Title":                 strings.Title,
		"PascalCase":            pascalCase,
		"SplitDisplayName":      gen.SplitDisplayName,
		"Contains":              strings.Contains,
		"HasPrefix":             strings.HasPrefix,
		"HasSuffix":             strings.HasSuffix,
		"TrimPrefix":            strings.TrimPrefix,
		"TrimSuffix":            strings.TrimSuffix,
		"TrimLeft":              strings.TrimLeft,
		"TrimRight":             strings.TrimRight,
		"Split":                 strings.Split,
		"Scramble":              g.scramble,
		"IsEnumEntry":           is(sem.EnumEntry{}),
		"IsEnumMatcher":         is(sem.EnumMatcher{}),
		"IsFQN":                 is(sem.FullyQualifiedName{}),
		"IsInt":                 is(1),
		"IsTemplateEnumParam":   is(sem.TemplateEnumParam{}),
		"IsTemplateNumberParam": is(sem.TemplateNumberParam{}),
		"IsTemplateTypeParam":   is(sem.TemplateTypeParam{}),
		"IsType":                is(sem.Type{}),
		"ElementType":           gen.ElementType,
		"DeepestElementType":    gen.DeepestElementType,
		"IsAbstract":            gen.IsAbstract,
		"IsDeclarable":          gen.IsDeclarable,
		"OverloadUsesF16":       gen.OverloadUsesF16,
		"IsFirstIn":             isFirstIn,
		"IsLastIn":              isLastIn,
		"Sem":                   g.cache.sem,
		"IntrinsicTable":        g.cache.intrinsicTable,
		"Permute":               g.cache.permute,
		"Eval":                  g.eval,
		"Import":                g.importTmpl,
		"WriteFile":             func(relPath, content string) (string, error) { return "", g.writeFile(relPath, content) },
	}).Option("missingkey=error").Parse(text)
	return err
}

// eval executes the sub-template with the given name and argument, returning
// the generated output
func (g *generator) eval(template string, args ...interface{}) (string, error) {
	target := g.template.Lookup(template)
	if target == nil {
		return "", fmt.Errorf("template '%v' not found", template)
	}
	sb := strings.Builder{}

	var err error
	if len(args) == 1 {
		err = target.Execute(&sb, args[0])
	} else {
		m := newMap()
		if len(args)%2 != 0 {
			return "", fmt.Errorf("Eval expects a single argument or list name-value pairs")
		}
		for i := 0; i < len(args); i += 2 {
			name, ok := args[i].(string)
			if !ok {
				return "", fmt.Errorf("Eval argument %v is not a string", i)
			}
			m.Put(name, args[i+1])
		}
		err = target.Execute(&sb, m)
	}

	if err != nil {
		return "", fmt.Errorf("while evaluating '%v': %v", template, err)
	}
	return sb.String(), nil
}

// importTmpl parses the template at the given project-relative path, merging
// the template definitions into the global namespace.
// Note: The body of the template is not executed.
func (g *generator) importTmpl(path string) (string, error) {
	if strings.Contains(path, "..") {
		return "", fmt.Errorf("import path must not contain '..'")
	}
	path = filepath.Join(fileutils.DawnRoot(), path)
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("failed to open '%v': %w", path, err)
	}
	t := g.template.New("")
	if err := g.bindAndParse(t, string(data)); err != nil {
		return "", fmt.Errorf("failed to parse '%v': %w", path, err)
	}
	return "", nil
}

// scramble randomly modifies the input string so that it is no longer equal to
// any of the strings in 'avoid'.
func (g *generator) scramble(str string, avoid container.Set[string]) (string, error) {
	bytes := []byte(str)
	passes := g.rnd.Intn(5) + 1

	const chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"

	char := func() byte { return chars[g.rnd.Intn(len(chars))] }
	replace := func(at int) { bytes[at] = char() }
	delete := func(at int) { bytes = append(bytes[:at], bytes[at+1:]...) }
	insert := func(at int) { bytes = append(append(bytes[:at], char()), bytes[at:]...) }

	for i := 0; i < passes || avoid.Contains(string(bytes)); i++ {
		if len(bytes) > 0 {
			at := g.rnd.Intn(len(bytes))
			switch g.rnd.Intn(3) {
			case 0:
				replace(at)
			case 1:
				delete(at)
			case 2:
				insert(at)
			}
		} else {
			insert(0)
		}
	}
	return string(bytes), nil
}

// Map is a simple generic key-value map, which can be used in the template
type Map map[interface{}]interface{}

func newMap() Map { return Map{} }

// Put adds the key-value pair into the map.
// Put always returns an empty string so nothing is printed in the template.
func (m Map) Put(key, value interface{}) string {
	m[key] = value
	return ""
}

// Get looks up and returns the value with the given key. If the map does not
// contain the given key, then nil is returned.
func (m Map) Get(key interface{}) interface{} {
	return m[key]
}

// is returns a function that returns true if the value passed to the function
// matches the type of 'ty'.
func is(ty interface{}) func(interface{}) bool {
	rty := reflect.TypeOf(ty)
	return func(v interface{}) bool {
		ty := reflect.TypeOf(v)
		return ty == rty || ty == reflect.PtrTo(rty)
	}
}

// isFirstIn returns true if v is the first element of the given slice.
func isFirstIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(0).Interface() == v
}

// isFirstIn returns true if v is the last element of the given slice.
func isLastIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(count-1).Interface() == v
}

// iterate returns a slice of length 'n', with each element equal to its index.
// Useful for: {{- range Iterate $n -}}<this will be looped $n times>{{end}}
func iterate(n int) []int {
	out := make([]int, n)
	for i := range out {
		out[i] = i
	}
	return out
}

// pascalCase returns the snake-case string s transformed into 'PascalCase',
// Rules:
// * The first letter of the string is capitalized
// * Characters following an underscore or number are capitalized
// * Underscores are removed from the returned string
// See: https://en.wikipedia.org/wiki/Camel_case
func pascalCase(s string) string {
	b := strings.Builder{}
	upper := true
	for _, r := range s {
		if r == '_' {
			upper = true
			continue
		}
		if upper {
			b.WriteRune(unicode.ToUpper(r))
			upper = false
		} else {
			b.WriteRune(r)
		}
		if unicode.IsNumber(r) {
			upper = true
		}
	}
	return b.String()
}

// Invokes the clang-format executable at 'exe' to format the file content 'in'.
// Returns the formatted file.
func clangFormat(in, exe string) (string, error) {
	cmd := exec.Command(exe)
	cmd.Stdin = strings.NewReader(in)
	out, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("clang-format failed:\n%v\n%v", string(out), err)
	}
	return string(out), nil
}

// Looks for clang-format in the 'buildtools' directory, falling back to PATH
func findClangFormat(projectRoot string) string {
	var path string
	switch runtime.GOOS {
	case "linux":
		path = filepath.Join(projectRoot, "buildtools/linux64/clang-format")
	case "darwin":
		path = filepath.Join(projectRoot, "buildtools/mac/clang-format")
	case "windows":
		path = filepath.Join(projectRoot, "buildtools/win/clang-format.exe")
	}
	if fileutils.IsExe(path) {
		return path
	}
	var err error
	path, err = exec.LookPath("clang-format")
	if err == nil {
		return path
	}
	return ""
}
