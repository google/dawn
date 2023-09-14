SKIP: FAILED


enable chromium_experimental_subgroups;

fn subgroupBroadcast_867093() {
  var arg_0 = 1.0f;
  const arg_1 = 1i;
  var res : f32 = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_867093();
}

Failed to generate: test/tint/builtins/gen/var/subgroupBroadcast/867093.wgsl:25:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

