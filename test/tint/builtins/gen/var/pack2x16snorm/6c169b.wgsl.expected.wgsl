fn pack2x16snorm_6c169b() {
  var arg_0 = vec2<f32>(1.0f);
  var res : u32 = pack2x16snorm(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pack2x16snorm_6c169b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pack2x16snorm_6c169b();
}

@compute @workgroup_size(1)
fn compute_main() {
  pack2x16snorm_6c169b();
}
