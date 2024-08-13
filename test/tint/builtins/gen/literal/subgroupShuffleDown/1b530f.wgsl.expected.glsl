SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupShuffleDown_1b530f() -> vec3<i32> {
  var res : vec3<i32> = subgroupShuffleDown(vec3<i32>(1i), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_1b530f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_1b530f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/1b530f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupShuffleDown_1b530f() -> vec3<i32> {
  var res : vec3<i32> = subgroupShuffleDown(vec3<i32>(1i), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_1b530f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_1b530f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleDown/1b530f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

