fn distance_cfed73() {
  var res : f32 = distance(1.0f, 1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_cfed73();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_cfed73();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_cfed73();
}
