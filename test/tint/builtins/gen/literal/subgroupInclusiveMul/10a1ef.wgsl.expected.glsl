SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupInclusiveMul_10a1ef() -> f16 {
  var res : f16 = subgroupInclusiveMul(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_10a1ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_10a1ef();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveMul/10a1ef.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupInclusiveMul_10a1ef() -> f16 {
  var res : f16 = subgroupInclusiveMul(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_10a1ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_10a1ef();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveMul/10a1ef.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
