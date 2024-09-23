SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupBroadcast_cd7aa1() -> vec2<f32> {
  var res : vec2<f32> = subgroupBroadcast(vec2<f32>(1.0f), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_cd7aa1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_cd7aa1();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/cd7aa1.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupBroadcast_cd7aa1() -> vec2<f32> {
  var res : vec2<f32> = subgroupBroadcast(vec2<f32>(1.0f), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_cd7aa1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_cd7aa1();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/cd7aa1.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
