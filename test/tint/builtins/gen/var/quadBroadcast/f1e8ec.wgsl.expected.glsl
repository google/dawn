SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadBroadcast_f1e8ec() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  const arg_1 = 1i;
  var res : vec3<u32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f1e8ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f1e8ec();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/f1e8ec.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadBroadcast_f1e8ec() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  const arg_1 = 1i;
  var res : vec3<u32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f1e8ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f1e8ec();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadBroadcast/f1e8ec.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

