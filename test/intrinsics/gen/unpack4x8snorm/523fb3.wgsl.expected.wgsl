fn unpack4x8snorm_523fb3() {
  var res : vec4<f32> = unpack4x8snorm(1u);
}

[[stage(vertex)]]
fn vertex_main() {
  unpack4x8snorm_523fb3();
}

[[stage(fragment)]]
fn fragment_main() {
  unpack4x8snorm_523fb3();
}

[[stage(compute)]]
fn compute_main() {
  unpack4x8snorm_523fb3();
}
