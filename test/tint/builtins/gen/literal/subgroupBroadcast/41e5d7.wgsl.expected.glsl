SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupBroadcast_41e5d7() -> vec3<f16> {
  var res : vec3<f16> = subgroupBroadcast(vec3<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_41e5d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_41e5d7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/41e5d7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupBroadcast_41e5d7() -> vec3<f16> {
  var res : vec3<f16> = subgroupBroadcast(vec3<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_41e5d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_41e5d7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/41e5d7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
