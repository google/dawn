SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupInclusiveMul_7978b8() -> vec3<f32> {
  var res : vec3<f32> = subgroupInclusiveMul(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_7978b8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_7978b8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveMul/7978b8.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupInclusiveMul_7978b8() -> vec3<f32> {
  var res : vec3<f32> = subgroupInclusiveMul(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_7978b8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_7978b8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveMul/7978b8.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
