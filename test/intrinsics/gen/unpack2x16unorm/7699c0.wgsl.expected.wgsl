fn unpack2x16unorm_7699c0() {
  var res : vec2<f32> = unpack2x16unorm(1u);
}

[[stage(vertex)]]
fn vertex_main() {
  unpack2x16unorm_7699c0();
}

[[stage(fragment)]]
fn fragment_main() {
  unpack2x16unorm_7699c0();
}

[[stage(compute)]]
fn compute_main() {
  unpack2x16unorm_7699c0();
}
