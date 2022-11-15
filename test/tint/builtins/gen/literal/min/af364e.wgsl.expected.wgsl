fn min_af364e() {
  var res = min(vec4(1), vec4(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_af364e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_af364e();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_af364e();
}
