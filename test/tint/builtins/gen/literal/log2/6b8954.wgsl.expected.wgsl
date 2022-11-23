fn log2_6b8954() {
  var res = log2(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_6b8954();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_6b8954();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_6b8954();
}
