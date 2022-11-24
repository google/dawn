fn distance_f9c9ee() {
  var res = distance(1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_f9c9ee();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_f9c9ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_f9c9ee();
}
