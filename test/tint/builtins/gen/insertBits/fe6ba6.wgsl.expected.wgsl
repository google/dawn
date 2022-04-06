fn insertBits_fe6ba6() {
  var res : vec2<i32> = insertBits(vec2<i32>(), vec2<i32>(), 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_fe6ba6();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_fe6ba6();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_fe6ba6();
}
