fn tanh_5724b3() {
  var res : vec2<f32> = tanh(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_5724b3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_5724b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_5724b3();
}
