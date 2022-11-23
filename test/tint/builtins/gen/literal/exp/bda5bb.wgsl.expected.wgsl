fn exp_bda5bb() {
  var res = exp(vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_bda5bb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_bda5bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_bda5bb();
}
