def _local_symlink_repo_impl(ctx):
    workspace_root = ctx.path(Label("@//:BUILD.bazel")).dirname
    target_path = workspace_root.get_child(ctx.attr.path)

    res = ctx.execute(["ls", str(target_path)])
    if res.return_code != 0:
        fail("Failed to list directory " + str(target_path))

    entries = res.stdout.splitlines()
    for entry in entries:
        if entry == "" or entry == "MODULE.bazel":
            continue
        ctx.symlink(target_path.get_child(entry), entry)

    ctx.file("MODULE.bazel", """
module(name = "spirv_tools")
bazel_dep(name = "rules_python", version = "1.5.1")
bazel_dep(name = "bazel_skylib", version = "1.5.0")
bazel_dep(name = "rules_cc", version = "0.1.1")
bazel_dep(name = "spirv_headers", version = "0.0.0")
bazel_dep(name = "platforms", version = "0.0.10")
""")

local_symlink_repo = repository_rule(
    implementation = _local_symlink_repo_impl,
    attrs = {
        "path": attr.string(mandatory = True),
    },
)

def _spirv_tools_repo_impl(ctx):
    local_symlink_repo(
        name = "spirv_tools",
        path = "third_party/spirv-tools/src",
    )

spirv_tools_repo = module_extension(
    implementation = _spirv_tools_repo_impl,
)
