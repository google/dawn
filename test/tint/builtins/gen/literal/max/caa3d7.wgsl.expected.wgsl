fn max_caa3d7() {
  var res = max(1, 1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_caa3d7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_caa3d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_caa3d7();
}
