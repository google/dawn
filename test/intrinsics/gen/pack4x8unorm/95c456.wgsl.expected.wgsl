fn pack4x8unorm_95c456() {
  var res : u32 = pack4x8unorm(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  pack4x8unorm_95c456();
}

[[stage(fragment)]]
fn fragment_main() {
  pack4x8unorm_95c456();
}

[[stage(compute)]]
fn compute_main() {
  pack4x8unorm_95c456();
}
