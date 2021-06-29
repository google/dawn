fn abs_7326de() {
  var res : vec3<u32> = abs(vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_7326de();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_7326de();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  abs_7326de();
}
