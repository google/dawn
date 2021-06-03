SKIP: FAILED


fn select_80a9a9() {
  var res : vec3<bool> = select(vec3<bool>(), vec3<bool>(), vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_80a9a9();
}

[[stage(fragment)]]
fn fragment_main() {
  select_80a9a9();
}

[[stage(compute)]]
fn compute_main() {
  select_80a9a9();
}

Failed to generate: error: select not supported in HLSL backend yet
