fn pack2x16float_0e97b3() {
  var res : u32 = pack2x16float(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  pack2x16float_0e97b3();
}

[[stage(fragment)]]
fn fragment_main() {
  pack2x16float_0e97b3();
}

[[stage(compute)]]
fn compute_main() {
  pack2x16float_0e97b3();
}
