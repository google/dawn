SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupMax_a3afe3() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_a3afe3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_a3afe3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMax/a3afe3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupMax_a3afe3() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_a3afe3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_a3afe3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMax/a3afe3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
