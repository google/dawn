SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn quadBroadcast_76f499() -> vec4<i32> {
  var res : vec4<i32> = quadBroadcast(vec4<i32>(1i), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_76f499();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_76f499();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/76f499.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn quadBroadcast_76f499() -> vec4<i32> {
  var res : vec4<i32> = quadBroadcast(vec4<i32>(1i), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_76f499();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_76f499();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/76f499.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

