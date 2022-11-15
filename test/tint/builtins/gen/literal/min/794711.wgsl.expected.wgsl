fn min_794711() {
  var res = min(1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_794711();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_794711();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_794711();
}
