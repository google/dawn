fn clamp_23aa4f() {
  var res = clamp(1.0, 1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_23aa4f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_23aa4f();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_23aa4f();
}
