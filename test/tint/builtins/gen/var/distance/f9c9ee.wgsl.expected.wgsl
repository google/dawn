fn distance_f9c9ee() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var res = distance(arg_0, arg_1);
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
