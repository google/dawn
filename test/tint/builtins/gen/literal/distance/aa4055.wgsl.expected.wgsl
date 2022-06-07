fn distance_aa4055() {
  var res : f32 = distance(vec2<f32>(), vec2<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_aa4055();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_aa4055();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_aa4055();
}
