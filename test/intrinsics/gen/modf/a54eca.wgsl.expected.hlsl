SKIP: FAILED


fn modf_a54eca() {
  var arg_1 : vec2<f32>;
  var res : vec2<f32> = modf(vec2<f32>(), &(arg_1));
}

struct tint_symbol {
  [[builtin(position)]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn vertex_main() -> tint_symbol {
  modf_a54eca();
  let tint_symbol_1 : tint_symbol = tint_symbol(vec4<f32>());
  return tint_symbol_1;
}

[[stage(fragment)]]
fn fragment_main() {
  modf_a54eca();
}

[[stage(compute)]]
fn compute_main() {
  modf_a54eca();
}

Failed to generate: error: Unknown builtin method: modf
