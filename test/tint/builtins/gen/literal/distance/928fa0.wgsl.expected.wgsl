enable f16;

fn distance_928fa0() {
  var res : f16 = distance(vec3<f16>(f16()), vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_928fa0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_928fa0();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_928fa0();
}
