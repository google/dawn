SKIP: FAILED


fn select_01e2cd() {
  var res : vec3<i32> = select(vec3<i32>(), vec3<i32>(), vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_01e2cd();
}

[[stage(fragment)]]
fn fragment_main() {
  select_01e2cd();
}

[[stage(compute)]]
fn compute_main() {
  select_01e2cd();
}

Failed to generate: error: select not supported in HLSL backend yet
