SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupShuffleDown_9c6714() -> f16 {
  var res : f16 = subgroupShuffleDown(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_9c6714();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_9c6714();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/9c6714.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupShuffleDown_9c6714() -> f16 {
  var res : f16 = subgroupShuffleDown(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_9c6714();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_9c6714();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/9c6714.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

