SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupShuffleXor_1d36b6() -> f32 {
  var res : f32 = subgroupShuffleXor(1.0f, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_1d36b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_1d36b6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/1d36b6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupShuffleXor_1d36b6() -> f32 {
  var res : f32 = subgroupShuffleXor(1.0f, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_1d36b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_1d36b6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/1d36b6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

