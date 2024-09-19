SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupBroadcast_1d79c7() -> i32 {
  var arg_0 = 1i;
  const arg_1 = 1u;
  var res : i32 = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_1d79c7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_1d79c7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/1d79c7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupBroadcast_1d79c7() -> i32 {
  var arg_0 = 1i;
  const arg_1 = 1u;
  var res : i32 = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_1d79c7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_1d79c7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/1d79c7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
