SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupMin_2493ab() -> u32 {
  var res : u32 = subgroupMin(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_2493ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_2493ab();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/2493ab.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupMin_2493ab() -> u32 {
  var res : u32 = subgroupMin(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_2493ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_2493ab();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/2493ab.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
