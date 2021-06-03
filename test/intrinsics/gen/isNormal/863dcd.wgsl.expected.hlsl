SKIP: FAILED


fn isNormal_863dcd() {
  var res : vec4<bool> = isNormal(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isNormal_863dcd();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_863dcd();
}

[[stage(compute)]]
fn compute_main() {
  isNormal_863dcd();
}

Failed to generate: error: is_normal not supported in HLSL backend yet
