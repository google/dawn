SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffleUp_3242a6() -> u32 {
  var res : u32 = subgroupShuffleUp(1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_3242a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_3242a6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/3242a6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffleUp_3242a6() -> u32 {
  var res : u32 = subgroupShuffleUp(1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_3242a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_3242a6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/3242a6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

