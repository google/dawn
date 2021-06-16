SKIP: FAILED


fn isNormal_b00ab1() {
  var res : vec2<bool> = isNormal(vec2<f32>());
}

struct tint_symbol {
  [[builtin(position)]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn vertex_main() -> tint_symbol {
  isNormal_b00ab1();
  let tint_symbol_1 : tint_symbol = tint_symbol(vec4<f32>());
  return tint_symbol_1;
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
