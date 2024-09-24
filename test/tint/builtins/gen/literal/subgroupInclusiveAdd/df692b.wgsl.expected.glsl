SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupInclusiveAdd_df692b() -> f32 {
  var res : f32 = subgroupInclusiveAdd(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_df692b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_df692b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveAdd/df692b.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupInclusiveAdd_df692b() -> f32 {
  var res : f32 = subgroupInclusiveAdd(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_df692b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_df692b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveAdd/df692b.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
