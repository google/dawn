fn dot_7548a0() {
  var res : u32 = dot(vec3<u32>(), vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  dot_7548a0();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_7548a0();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  dot_7548a0();
}
