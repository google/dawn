SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupInclusiveAdd_58ea3d() -> vec4<f16> {
  var res : vec4<f16> = subgroupInclusiveAdd(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_58ea3d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_58ea3d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveAdd/58ea3d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupInclusiveAdd_58ea3d() -> vec4<f16> {
  var res : vec4<f16> = subgroupInclusiveAdd(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_58ea3d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_58ea3d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveAdd/58ea3d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
