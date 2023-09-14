SKIP: FAILED


enable chromium_experimental_subgroups;

fn subgroupBroadcast_49de94() {
  var arg_0 = 1u;
  const arg_1 = 1i;
  var res : u32 = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_49de94();
}

Failed to generate: test/tint/builtins/gen/var/subgroupBroadcast/49de94.wgsl:25:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

