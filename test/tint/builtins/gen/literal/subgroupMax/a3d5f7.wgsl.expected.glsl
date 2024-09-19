SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupMax_a3d5f7() -> vec4<i32> {
  var res : vec4<i32> = subgroupMax(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_a3d5f7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_a3d5f7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/a3d5f7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupMax_a3d5f7() -> vec4<i32> {
  var res : vec4<i32> = subgroupMax(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_a3d5f7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_a3d5f7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/a3d5f7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
