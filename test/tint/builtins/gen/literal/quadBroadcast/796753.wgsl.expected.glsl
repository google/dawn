SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn quadBroadcast_796753() -> vec3<f16> {
  var res : vec3<f16> = quadBroadcast(vec3<f16>(1.0h), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_796753();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_796753();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/796753.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn quadBroadcast_796753() -> vec3<f16> {
  var res : vec3<f16> = quadBroadcast(vec3<f16>(1.0h), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_796753();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_796753();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/796753.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

