fn tanh_313aa1() {
  var res = tanh(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_313aa1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_313aa1();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_313aa1();
}
