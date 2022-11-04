fn clamp_87df46() {
  var res = clamp(vec4(1.0), vec4(1.0), vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_87df46();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_87df46();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_87df46();
}
