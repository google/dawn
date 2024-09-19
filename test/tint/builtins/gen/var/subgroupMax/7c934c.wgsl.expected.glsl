SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupMax_7c934c() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_7c934c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_7c934c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMax/7c934c.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupMax_7c934c() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_7c934c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_7c934c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMax/7c934c.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
