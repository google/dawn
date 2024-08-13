SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffleDown_d269eb() -> i32 {
  var res : i32 = subgroupShuffleDown(1i, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_d269eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_d269eb();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/d269eb.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffleDown_d269eb() -> i32 {
  var res : i32 = subgroupShuffleDown(1i, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_d269eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_d269eb();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/d269eb.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

