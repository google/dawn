SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupMax_4ea90e() -> vec3<i32> {
  var res : vec3<i32> = subgroupMax(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_4ea90e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_4ea90e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/4ea90e.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupMax_4ea90e() -> vec3<i32> {
  var res : vec3<i32> = subgroupMax(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_4ea90e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_4ea90e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/4ea90e.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
