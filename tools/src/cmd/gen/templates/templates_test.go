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

func TestCmd_Run_FileDiscovery(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	// Create the template file
	// Note: The glob pattern in templates.go is "src/tint/**.tmpl" and "test/tint/**.tmpl"
	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "test.tmpl")
	tmplContent := `Test Template 1`
	err := osw.MkdirAll(filepath.Dir(tmplPath), 0755)
	require.NoError(t, err, "Failed to create tmpl dir")

	err = osw.WriteFile(tmplPath, []byte(tmplContent), 0644)
	require.NoError(t, err, "Failed to write tmpl")

	// Create a second template file in the test directory to verify the other glob pattern
	tmplPath2 := filepath.Join(realDawnRoot, "test", "tint", "subdir", "test2.tmpl")
	tmplContent2 := `Test Template 2`
	err = osw.MkdirAll(filepath.Dir(tmplPath2), 0755)
	require.NoError(t, err, "Failed to create tmpl2 dir")
	err = osw.WriteFile(tmplPath2, []byte(tmplContent2), 0644)
	require.NoError(t, err, "Failed to write tmpl2")

	c := &Cmd{}
	err = c.Run(ctx, cfg)
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

	// Create the template file
	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "explicit.tmpl")
	tmplContent := `Explicit Template`
	err := osw.MkdirAll(filepath.Dir(tmplPath), 0755)
	require.NoError(t, err, "Failed to create tmpl dir")
	err = osw.WriteFile(tmplPath, []byte(tmplContent), 0644)
	require.NoError(t, err, "Failed to write tmpl")

	// Inject args since the existing FlagSet's args cannot be easily modified.
	// NOTE: This means that this test is incompatible with t.Parallel() since it
	// is modifying global state.
	args := []string{tmplPath}
	origCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = origCommandLine }()

	flag.CommandLine = flag.NewFlagSet("test", flag.ContinueOnError)
	err = flag.CommandLine.Parse(args)
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
