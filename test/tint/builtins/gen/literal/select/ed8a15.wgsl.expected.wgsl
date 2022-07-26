fn select_ed8a15() {
  var res : i32 = select(1, 1, true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_ed8a15();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_ed8a15();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_ed8a15();
}
