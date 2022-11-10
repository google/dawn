fn tanh_c48aa6() {
  var res = tanh(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_c48aa6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_c48aa6();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_c48aa6();
}
