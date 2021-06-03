SKIP: FAILED


fn select_bb8aae() {
  var res : vec4<f32> = select(vec4<f32>(), vec4<f32>(), vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_bb8aae();
}

[[stage(fragment)]]
fn fragment_main() {
  select_bb8aae();
}

[[stage(compute)]]
fn compute_main() {
  select_bb8aae();
}

Failed to generate: error: select not supported in HLSL backend yet
