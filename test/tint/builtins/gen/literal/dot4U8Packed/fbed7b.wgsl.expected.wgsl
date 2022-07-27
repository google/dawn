enable chromium_experimental_dp4a;

fn dot4U8Packed_fbed7b() {
  var res : u32 = dot4U8Packed(1u, 1u);
}

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
