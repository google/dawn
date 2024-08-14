SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn quadBroadcast_3c3824() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec2<f16> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_3c3824();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_3c3824();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/3c3824.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn quadBroadcast_3c3824() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec2<f16> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_3c3824();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_3c3824();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/3c3824.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

