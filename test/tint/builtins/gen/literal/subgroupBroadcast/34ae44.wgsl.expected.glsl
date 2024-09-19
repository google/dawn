SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupBroadcast_34ae44() -> vec3<u32> {
  var res : vec3<u32> = subgroupBroadcast(vec3<u32>(1u), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_34ae44();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_34ae44();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/34ae44.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupBroadcast_34ae44() -> vec3<u32> {
  var res : vec3<u32> = subgroupBroadcast(vec3<u32>(1u), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_34ae44();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_34ae44();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/34ae44.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
