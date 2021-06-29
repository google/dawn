fn pack2x16snorm_6c169b() {
  var res : u32 = pack2x16snorm(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  pack2x16snorm_6c169b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  pack2x16snorm_6c169b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  pack2x16snorm_6c169b();
}
