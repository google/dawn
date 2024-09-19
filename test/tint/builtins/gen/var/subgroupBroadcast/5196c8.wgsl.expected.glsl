SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupBroadcast_5196c8() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  const arg_1 = 1u;
  var res : vec2<f32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_5196c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_5196c8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/5196c8.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupBroadcast_5196c8() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  const arg_1 = 1u;
  var res : vec2<f32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_5196c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_5196c8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/5196c8.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
