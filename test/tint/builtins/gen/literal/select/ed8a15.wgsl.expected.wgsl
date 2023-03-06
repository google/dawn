fn select_ed8a15() {
  var res : i32 = select(1i, 1i, true);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

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
