fn min_98e797() {
  var res = min(vec4(1.0), vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_98e797();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_98e797();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_98e797();
}
