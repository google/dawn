fn clamp_a2de25() {
  var res : u32 = clamp(1u, 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

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
