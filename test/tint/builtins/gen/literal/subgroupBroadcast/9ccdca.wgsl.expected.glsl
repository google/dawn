SKIP: FAILED


enable chromium_experimental_subgroups;

fn subgroupBroadcast_9ccdca() {
  var res : i32 = subgroupBroadcast(1i, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_9ccdca();
}

Failed to generate: test/tint/builtins/gen/literal/subgroupBroadcast/9ccdca.wgsl:25:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

