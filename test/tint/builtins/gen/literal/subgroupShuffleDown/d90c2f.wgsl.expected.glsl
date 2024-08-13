SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffleDown_d90c2f() -> u32 {
  var res : u32 = subgroupShuffleDown(1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_d90c2f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_d90c2f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/d90c2f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffleDown_d90c2f() -> u32 {
  var res : u32 = subgroupShuffleDown(1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_d90c2f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_d90c2f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/d90c2f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

