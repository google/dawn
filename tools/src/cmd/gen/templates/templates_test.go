package templates

import (
	"context"
	"flag"
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func setupRunFileTest(t *testing.T) (oswrapper.FSTestOSWrapper, *common.Config, string) {
	t.Helper()

	// Identify the "real" DawnRoot so we can populate the mock FS such that
	// fileutils.DawnRoot(mockFS) returns it.
	realOS := oswrapper.GetRealOSWrapper()
	realDawnRoot := fileutils.DawnRoot(realOS)
	require.NotEmpty(t, realDawnRoot, "Could not determine real DawnRoot")

	osw := oswrapper.CreateFSTestOSWrapper()
	cfg := &common.Config{
		OsWrapper: osw,
	}

	// Populate mock FS with DEPS at realDawnRoot so DawnRoot(osw) works.
	err := osw.MkdirAll(realDawnRoot, 0755)
	require.NoError(t, err, "Failed to create root in mock FS")
	err = osw.WriteFile(filepath.Join(realDawnRoot, "DEPS"), []byte(""), 0644)
	require.NoError(t, err, "Failed to write DEPS")

	// Create the directory structure corresponding to ThisDir() to ensure path lookup works.
	thisDir := fileutils.ThisDir()
	err = osw.MkdirAll(thisDir, 0755)
	require.NoError(t, err, "Failed to create ThisDir in mock FS")

	return osw, cfg, realDawnRoot
}

func createTemplateFile(t *testing.T, osw oswrapper.FilesystemWriter, path, content string) {
	t.Helper()
	err := osw.MkdirAll(filepath.Dir(path), 0755)
	require.NoError(t, err, "Failed to create directory for template")
	err = osw.WriteFile(path, []byte(content), 0644)
	require.NoError(t, err, "Failed to write template file")
}

func TestCmd_Run_FileDiscovery(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	// Create the template files
	// Note: The glob pattern in templates.go is "src/tint/**.tmpl" and "test/tint/**.tmpl"
	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "test.tmpl")
	createTemplateFile(t, osw, tmplPath, `Test Template 1`)
	tmplPath2 := filepath.Join(realDawnRoot, "test", "tint", "subdir", "test2.tmpl")
	createTemplateFile(t, osw, tmplPath2, `Test Template 2`)

	c := &Cmd{}
	err := c.Run(ctx, cfg)
	require.NoError(t, err, "Run failed")

	// Verify output 1
	outPath := filepath.Join(realDawnRoot, "src", "tint", "test")
	content, err := osw.ReadFile(outPath)
	require.NoError(t, err, "Output file 1 not found")
	require.Contains(t, string(content), "Test Template 1", "Output content 1 mismatch")

	// Verify output 2
	outPath2 := filepath.Join(realDawnRoot, "test", "tint", "subdir", "test2")
	content2, err := osw.ReadFile(outPath2)
	require.NoError(t, err, "Output file 2 not found")
	require.Contains(t, string(content2), "Test Template 2", "Output content 2 mismatch")
}

func TestCmd_Run_ExplicitFiles(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "explicit.tmpl")
	createTemplateFile(t, osw, tmplPath, `Explicit Template`)

	// Inject args since the existing FlagSet's args cannot be easily modified.
	// NOTE: This means that this test is incompatible with t.Parallel() since it
	// is modifying global state.
	args := []string{tmplPath}
	origCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = origCommandLine }()

	flag.CommandLine = flag.NewFlagSet("test", flag.ContinueOnError)
	err := flag.CommandLine.Parse(args)
	require.NoError(t, err, "Failed to parse mock flags")

	c := &Cmd{}
	err = c.Run(ctx, cfg)
	require.NoError(t, err, "Run failed")

	// Verify output
	outPath := filepath.Join(realDawnRoot, "src", "tint", "explicit")
	content, err := osw.ReadFile(outPath)
	require.NoError(t, err, "Output file not found")

	// Check content
	require.Contains(t, string(content), "Explicit Template", "Output content mismatch")
}

func TestCmd_Run_StaleCheck(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "stale.tmpl")
	createTemplateFile(t, osw, tmplPath, `Stale Template`)

	// Create the output file with different content
	outPath := filepath.Join(realDawnRoot, "src", "tint", "stale")
	err := osw.WriteFile(outPath, []byte("Different Content"), 0644)
	require.NoError(t, err, "Failed to write existing output file")

	cfg.Flags.CheckStale = true
	c := &Cmd{}
	err = c.Run(ctx, cfg)

	require.Error(t, err, "Run should have returned an error for stale files")
	staleFiles, ok := err.(common.StaleFiles)
	require.True(t, ok, "Error should be of type common.StaleFiles")
	require.Len(t, staleFiles, 1, "Should have 1 stale file")
	require.Equal(t, outPath, staleFiles[0], "Stale file path mismatch")
}

func TestCmd_Run_StaleCheck_Clean(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "clean.tmpl")
	createTemplateFile(t, osw, tmplPath, `Clean Template`)

	// First run: generate the file
	c := &Cmd{}
	err := c.Run(ctx, cfg)
	require.NoError(t, err, "First run failed")

	// Verify the file exists
	outPath := filepath.Join(realDawnRoot, "src", "tint", "clean")
	_, err = osw.ReadFile(outPath)
	require.NoError(t, err, "Output file should exist after first run")

	// Second run: check for staleness
	cfg.Flags.CheckStale = true
	err = c.Run(ctx, cfg)
	require.NoError(t, err, "Second run (stale check) should not return error for clean file")
}
