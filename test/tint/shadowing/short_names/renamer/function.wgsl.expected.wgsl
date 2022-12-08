fn tint_symbol() -> i32 {
  return 0;
}

fn tint_symbol_1(tint_symbol_2 : i32) -> f32 {
  return f32(tint_symbol_2);
}

fn tint_symbol_3(tint_symbol_4 : f32) -> bool {
  return bool(tint_symbol_4);
}

@vertex
fn tint_symbol_5(@builtin(vertex_index) tint_symbol_6 : u32) -> @builtin(position) vec4<f32> {
  return select(vec4<f32>(), vec4<f32>(1), tint_symbol_3(tint_symbol_1(tint_symbol())));
}
