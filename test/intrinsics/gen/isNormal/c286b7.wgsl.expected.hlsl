SKIP: FAILED


fn isNormal_c286b7() {
  var res : vec3<bool> = isNormal(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isNormal_c286b7();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_c286b7();
}

[[stage(compute)]]
fn compute_main() {
  isNormal_c286b7();
}

Failed to generate: error: is_normal not supported in HLSL backend yet
