fn exp2_dea523() {
  var arg_0 = 1.0f;
  var res : f32 = exp2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_dea523();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_dea523();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_dea523();
}
