SKIP: FAILED


fn select_1e960b() {
  var res : vec2<u32> = select(vec2<u32>(), vec2<u32>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_1e960b();
}

[[stage(fragment)]]
fn fragment_main() {
  select_1e960b();
}

[[stage(compute)]]
fn compute_main() {
  select_1e960b();
}

Failed to generate: error: select not supported in HLSL backend yet
