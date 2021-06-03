SKIP: FAILED


fn select_28a27e() {
  var res : vec3<u32> = select(vec3<u32>(), vec3<u32>(), vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_28a27e();
}

[[stage(fragment)]]
fn fragment_main() {
  select_28a27e();
}

[[stage(compute)]]
fn compute_main() {
  select_28a27e();
}

Failed to generate: error: select not supported in HLSL backend yet
