fn distance_ac5535() {
  var res = distance(vec4(1.0), vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_ac5535();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_ac5535();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_ac5535();
}
