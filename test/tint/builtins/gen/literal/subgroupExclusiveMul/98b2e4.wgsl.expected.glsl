SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupExclusiveMul_98b2e4() -> f32 {
  var res : f32 = subgroupExclusiveMul(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_98b2e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_98b2e4();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveMul/98b2e4.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupExclusiveMul_98b2e4() -> f32 {
  var res : f32 = subgroupExclusiveMul(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_98b2e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_98b2e4();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveMul/98b2e4.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
