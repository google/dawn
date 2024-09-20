SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupInclusiveAdd_7f2040() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_7f2040();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_7f2040();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/7f2040.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupInclusiveAdd_7f2040() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_7f2040();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_7f2040();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/7f2040.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
