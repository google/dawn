fn abs_7326de() {
  var res : vec3<u32> = abs(vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  abs_7326de();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_7326de();
}

[[stage(compute)]]
fn compute_main() {
  abs_7326de();
}
