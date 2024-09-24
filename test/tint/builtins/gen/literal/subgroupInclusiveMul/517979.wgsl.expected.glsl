SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupInclusiveMul_517979() -> vec4<i32> {
  var res : vec4<i32> = subgroupInclusiveMul(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_517979();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_517979();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveMul/517979.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupInclusiveMul_517979() -> vec4<i32> {
  var res : vec4<i32> = subgroupInclusiveMul(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_517979();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_517979();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveMul/517979.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
