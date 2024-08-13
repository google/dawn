SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupShuffleUp_8a63f3() -> vec3<i32> {
  var res : vec3<i32> = subgroupShuffleUp(vec3<i32>(1i), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_8a63f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_8a63f3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/8a63f3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupShuffleUp_8a63f3() -> vec3<i32> {
  var res : vec3<i32> = subgroupShuffleUp(vec3<i32>(1i), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_8a63f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_8a63f3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/8a63f3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

