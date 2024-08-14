SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadBroadcast_f5f923() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  const arg_1 = 1u;
  var res : vec2<i32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f5f923();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f5f923();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/f5f923.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadBroadcast_f5f923() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  const arg_1 = 1u;
  var res : vec2<i32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f5f923();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f5f923();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/f5f923.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

