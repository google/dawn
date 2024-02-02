SKIP: FAILED


enable chromium_experimental_subgroups;
enable f16;

fn subgroupBroadcast_07e2d8() {
  var res : f16 = subgroupBroadcast(1.0h, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_07e2d8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/07e2d8.wgsl:41:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

