SKIP: FAILED


fn select_00b848() {
  var res : vec2<i32> = select(vec2<i32>(), vec2<i32>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_00b848();
}

[[stage(fragment)]]
fn fragment_main() {
  select_00b848();
}

[[stage(compute)]]
fn compute_main() {
  select_00b848();
}

Failed to generate: error: select not supported in HLSL backend yet
