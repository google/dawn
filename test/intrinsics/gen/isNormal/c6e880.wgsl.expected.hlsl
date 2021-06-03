SKIP: FAILED


fn isNormal_c6e880() {
  var res : bool = isNormal(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  isNormal_c6e880();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_c6e880();
}

[[stage(compute)]]
fn compute_main() {
  isNormal_c6e880();
}

Failed to generate: error: is_normal not supported in HLSL backend yet
