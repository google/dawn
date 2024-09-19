SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupMin_030ad6() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_030ad6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_030ad6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/030ad6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupMin_030ad6() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_030ad6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_030ad6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/030ad6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
