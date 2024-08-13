SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupShuffleUp_bbf7f4() -> f16 {
  var res : f16 = subgroupShuffleUp(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_bbf7f4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_bbf7f4();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/bbf7f4.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupShuffleUp_bbf7f4() -> f16 {
  var res : f16 = subgroupShuffleUp(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_bbf7f4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_bbf7f4();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/bbf7f4.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

