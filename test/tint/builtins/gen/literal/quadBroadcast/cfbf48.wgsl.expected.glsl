SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn quadBroadcast_cfbf48() -> vec2<f32> {
  var res : vec2<f32> = quadBroadcast(vec2<f32>(1.0f), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_cfbf48();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_cfbf48();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/cfbf48.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn quadBroadcast_cfbf48() -> vec2<f32> {
  var res : vec2<f32> = quadBroadcast(vec2<f32>(1.0f), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_cfbf48();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_cfbf48();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/cfbf48.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

