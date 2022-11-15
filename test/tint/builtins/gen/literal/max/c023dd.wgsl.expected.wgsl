fn max_c023dd() {
  var res = max(1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_c023dd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_c023dd();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_c023dd();
}
