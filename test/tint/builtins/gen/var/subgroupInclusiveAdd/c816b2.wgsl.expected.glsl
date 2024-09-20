SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupInclusiveAdd_c816b2() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_c816b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_c816b2();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/c816b2.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupInclusiveAdd_c816b2() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_c816b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_c816b2();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/c816b2.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
