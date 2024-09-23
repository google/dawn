SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupMax_7e81ea() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_7e81ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_7e81ea();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMax/7e81ea.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupMax_7e81ea() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_7e81ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_7e81ea();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMax/7e81ea.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
