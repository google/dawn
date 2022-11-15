fn min_527b79() {
  var res = min(vec2(1), vec2(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_527b79();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_527b79();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_527b79();
}
