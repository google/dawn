SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadBroadcast_f9d579() -> i32 {
  var arg_0 = 1i;
  const arg_1 = 1i;
  var res : i32 = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f9d579();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f9d579();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/f9d579.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadBroadcast_f9d579() -> i32 {
  var arg_0 = 1i;
  const arg_1 = 1i;
  var res : i32 = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f9d579();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f9d579();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/f9d579.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

