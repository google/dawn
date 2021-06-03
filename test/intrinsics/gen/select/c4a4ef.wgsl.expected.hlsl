SKIP: FAILED


fn select_c4a4ef() {
  var res : vec4<u32> = select(vec4<u32>(), vec4<u32>(), vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_c4a4ef();
}

[[stage(fragment)]]
fn fragment_main() {
  select_c4a4ef();
}

[[stage(compute)]]
fn compute_main() {
  select_c4a4ef();
}

Failed to generate: error: select not supported in HLSL backend yet
