SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupExclusiveMul_6f431e() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = subgroupExclusiveMul(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_6f431e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_6f431e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupExclusiveMul/6f431e.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupExclusiveMul_6f431e() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = subgroupExclusiveMul(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_6f431e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_6f431e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupExclusiveMul/6f431e.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
