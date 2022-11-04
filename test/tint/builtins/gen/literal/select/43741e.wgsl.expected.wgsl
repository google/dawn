fn select_43741e() {
  var res = select(vec4(1.0), vec4(1.0), vec4<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_43741e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_43741e();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_43741e();
}
