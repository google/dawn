fn exp_699629() {
  var res = exp(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_699629();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_699629();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_699629();
}
