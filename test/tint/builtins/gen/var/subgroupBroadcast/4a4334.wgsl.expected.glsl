SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupBroadcast_4a4334() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  const arg_1 = 1u;
  var res : vec2<u32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/4a4334.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupBroadcast_4a4334() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  const arg_1 = 1u;
  var res : vec2<u32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/4a4334.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
