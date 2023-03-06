fn clamp_b07c65() {
  var res : i32 = clamp(1i, 1i, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

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
