fn tanh_ac5d33() {
  var res = tanh(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_ac5d33();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_ac5d33();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_ac5d33();
}
