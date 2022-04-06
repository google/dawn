fn insertBits_428b0b() {
  var res : vec3<i32> = insertBits(vec3<i32>(), vec3<i32>(), 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_428b0b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_428b0b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_428b0b();
}
