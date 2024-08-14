SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn quadBroadcast_76f499() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  const arg_1 = 1i;
  var res : vec4<i32> = quadBroadcast(arg_0, arg_1);
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

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/76f499.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn quadBroadcast_76f499() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  const arg_1 = 1i;
  var res : vec4<i32> = quadBroadcast(arg_0, arg_1);
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

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/76f499.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

