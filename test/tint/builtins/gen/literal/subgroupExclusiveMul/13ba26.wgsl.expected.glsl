SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupExclusiveMul_13ba26() -> vec3<f16> {
  var res : vec3<f16> = subgroupExclusiveMul(vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_13ba26();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_13ba26();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveMul/13ba26.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupExclusiveMul_13ba26() -> vec3<f16> {
  var res : vec3<f16> = subgroupExclusiveMul(vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_13ba26();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_13ba26();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveMul/13ba26.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
