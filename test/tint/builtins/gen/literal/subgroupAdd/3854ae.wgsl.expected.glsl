SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupAdd_3854ae() -> f32 {
  var res : f32 = subgroupAdd(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_3854ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_3854ae();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/3854ae.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupAdd_3854ae() -> f32 {
  var res : f32 = subgroupAdd(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_3854ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_3854ae();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/3854ae.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
