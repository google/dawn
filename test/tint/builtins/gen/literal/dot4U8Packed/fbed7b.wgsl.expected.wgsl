enable chromium_experimental_dp4a;

fn dot4U8Packed_fbed7b() {
  var res : u32 = dot4U8Packed(1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot4U8Packed_fbed7b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot4U8Packed_fbed7b();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot4U8Packed_fbed7b();
}
