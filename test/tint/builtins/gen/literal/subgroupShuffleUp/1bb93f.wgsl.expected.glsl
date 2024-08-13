SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffleUp_1bb93f() -> i32 {
  var res : i32 = subgroupShuffleUp(1i, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_1bb93f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_1bb93f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/1bb93f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffleUp_1bb93f() -> i32 {
  var res : i32 = subgroupShuffleUp(1i, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_1bb93f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_1bb93f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/1bb93f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

