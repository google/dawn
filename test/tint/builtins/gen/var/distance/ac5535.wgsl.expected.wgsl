fn distance_ac5535() {
  const arg_0 = vec4(1.0);
  const arg_1 = vec4(1.0);
  var res = distance(arg_0, arg_1);
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
