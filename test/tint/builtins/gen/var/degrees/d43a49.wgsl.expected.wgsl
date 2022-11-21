fn degrees_d43a49() {
  const arg_0 = vec4(1.0);
  var res = degrees(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_d43a49();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_d43a49();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_d43a49();
}
