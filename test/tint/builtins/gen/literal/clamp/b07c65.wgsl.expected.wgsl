fn clamp_b07c65() {
  var res : i32 = clamp(1i, 1i, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_b07c65();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_b07c65();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_b07c65();
}
