fn pack2x16unorm_0f08e4() {
  var res : u32 = pack2x16unorm(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  pack2x16unorm_0f08e4();
}

[[stage(fragment)]]
fn fragment_main() {
  pack2x16unorm_0f08e4();
}

[[stage(compute)]]
fn compute_main() {
  pack2x16unorm_0f08e4();
}
