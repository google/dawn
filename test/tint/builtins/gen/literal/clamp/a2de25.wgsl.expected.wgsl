fn clamp_a2de25() {
  var res : u32 = clamp(1u, 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_a2de25();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_a2de25();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_a2de25();
}
