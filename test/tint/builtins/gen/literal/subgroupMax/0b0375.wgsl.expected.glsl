SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupMax_0b0375() -> vec4<f32> {
  var res : vec4<f32> = subgroupMax(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_0b0375();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_0b0375();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/0b0375.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupMax_0b0375() -> vec4<f32> {
  var res : vec4<f32> = subgroupMax(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_0b0375();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_0b0375();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/0b0375.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
