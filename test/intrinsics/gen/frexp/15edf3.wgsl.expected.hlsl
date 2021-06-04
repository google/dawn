SKIP: FAILED


fn frexp_15edf3() {
  var arg_1 : vec2<i32>;
  var res : vec2<f32> = frexp(vec2<f32>(), &(arg_1));
}

struct tint_symbol {
  [[builtin(position)]]
  value : vec4<f32>;
};

[[stage(vertex)]]
fn vertex_main() -> tint_symbol {
  frexp_15edf3();
  let tint_symbol_1 : tint_symbol = tint_symbol(vec4<f32>());
  return tint_symbol_1;
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_15edf3();
}

[[stage(compute)]]
fn compute_main() {
  frexp_15edf3();
}

Failed to generate: error: Unknown builtin method: frexp
