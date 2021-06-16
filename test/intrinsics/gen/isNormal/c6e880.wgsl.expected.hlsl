SKIP: FAILED


fn isNormal_c6e880() {
  var res : bool = isNormal(1.0);
}

struct tint_symbol {
  [[builtin(position)]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn vertex_main() -> tint_symbol {
  isNormal_c6e880();
  let tint_symbol_1 : tint_symbol = tint_symbol(vec4<f32>());
  return tint_symbol_1;
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
