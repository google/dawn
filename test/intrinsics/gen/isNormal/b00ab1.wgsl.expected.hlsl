SKIP: FAILED


fn isNormal_b00ab1() {
  var res : vec2<bool> = isNormal(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isNormal_b00ab1();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_b00ab1();
}

[[stage(compute)]]
fn compute_main() {
  isNormal_b00ab1();
}

Failed to generate: error: is_normal not supported in HLSL backend yet
