SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupMax_1a1a5f() -> f32 {
  var res : f32 = subgroupMax(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_1a1a5f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_1a1a5f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/1a1a5f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupMax_1a1a5f() -> f32 {
  var res : f32 = subgroupMax(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_1a1a5f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_1a1a5f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/1a1a5f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
