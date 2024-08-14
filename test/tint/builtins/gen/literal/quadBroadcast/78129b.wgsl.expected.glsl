SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadBroadcast_78129b() -> f16 {
  var res : f16 = quadBroadcast(1.0h, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_78129b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_78129b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/78129b.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadBroadcast_78129b() -> f16 {
  var res : f16 = quadBroadcast(1.0h, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_78129b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_78129b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadBroadcast/78129b.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^

