SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupAdd_ba53f9() -> i32 {
  var res : i32 = subgroupAdd(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_ba53f9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_ba53f9();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/ba53f9.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupAdd_ba53f9() -> i32 {
  var res : i32 = subgroupAdd(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_ba53f9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_ba53f9();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/ba53f9.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
