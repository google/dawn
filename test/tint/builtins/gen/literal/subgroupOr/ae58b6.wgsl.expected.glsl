SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupOr_ae58b6() -> i32 {
  var res : i32 = subgroupOr(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_ae58b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_ae58b6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupOr/ae58b6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupOr_ae58b6() -> i32 {
  var res : i32 = subgroupOr(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_ae58b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_ae58b6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupOr/ae58b6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
