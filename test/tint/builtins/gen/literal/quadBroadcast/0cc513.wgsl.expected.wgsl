enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn quadBroadcast_0cc513() -> vec3<f32> {
  var res : vec3<f32> = quadBroadcast(vec3<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_0cc513();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_0cc513();
}
