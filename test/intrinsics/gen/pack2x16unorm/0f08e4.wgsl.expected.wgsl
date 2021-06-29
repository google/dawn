fn pack2x16unorm_0f08e4() {
  var res : u32 = pack2x16unorm(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  pack2x16unorm_0f08e4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  pack2x16unorm_0f08e4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  pack2x16unorm_0f08e4();
}
