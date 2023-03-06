fn insertBits_e3e3a2() {
  var res : u32 = insertBits(1u, 1u, 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_e3e3a2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_e3e3a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_e3e3a2();
}
