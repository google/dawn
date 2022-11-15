fn min_84c9fe() {
  var res = min(1, 1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_84c9fe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_84c9fe();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_84c9fe();
}
