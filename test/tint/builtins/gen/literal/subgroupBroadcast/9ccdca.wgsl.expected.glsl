SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupBroadcast_9ccdca() -> i32 {
  var res : i32 = subgroupBroadcast(1i, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_9ccdca();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_9ccdca();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/9ccdca.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupBroadcast_9ccdca() -> i32 {
  var res : i32 = subgroupBroadcast(1i, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_9ccdca();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_9ccdca();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/9ccdca.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
