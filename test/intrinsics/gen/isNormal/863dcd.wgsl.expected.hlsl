SKIP: FAILED


fn isNormal_863dcd() {
  var res : vec4<bool> = isNormal(vec4<f32>());
}

struct tint_symbol {
  [[builtin(position)]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn vertex_main() -> tint_symbol {
  isNormal_863dcd();
  let tint_symbol_1 : tint_symbol = tint_symbol(vec4<f32>());
  return tint_symbol_1;
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
