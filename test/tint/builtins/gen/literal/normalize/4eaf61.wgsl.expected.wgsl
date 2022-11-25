fn normalize_4eaf61() {
  var res = normalize(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_4eaf61();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_4eaf61();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_4eaf61();
}
