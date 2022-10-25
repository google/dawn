fn insertBits_65468b() {
  var res : i32 = insertBits(1i, 1i, 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_65468b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_65468b();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_65468b();
}
