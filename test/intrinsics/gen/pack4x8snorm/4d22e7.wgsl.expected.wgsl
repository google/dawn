fn pack4x8snorm_4d22e7() {
  var res : u32 = pack4x8snorm(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  pack4x8snorm_4d22e7();
}

[[stage(fragment)]]
fn fragment_main() {
  pack4x8snorm_4d22e7();
}

[[stage(compute)]]
fn compute_main() {
  pack4x8snorm_4d22e7();
}
