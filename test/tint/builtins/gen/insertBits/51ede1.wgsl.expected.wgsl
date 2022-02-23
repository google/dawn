fn insertBits_51ede1() {
  var res : vec4<u32> = insertBits(vec4<u32>(), vec4<u32>(), 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_51ede1();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  insertBits_51ede1();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  insertBits_51ede1();
}
