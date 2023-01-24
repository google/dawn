struct tint_symbol {
  tint_symbol_1 : i32,
}

alias tint_symbol_2 = f32;

alias tint_symbol_3 = bool;

@vertex
fn tint_symbol_4(@builtin(vertex_index) tint_symbol_5 : u32) -> @builtin(position) vec4<f32> {
  let tint_symbol_6 = tint_symbol(1);
  let tint_symbol_7 : f32 = tint_symbol_2(tint_symbol_6.tint_symbol_1);
  let tint_symbol_8 : bool = tint_symbol_3(tint_symbol_7);
  return select(vec4<f32>(), vec4<f32>(1), tint_symbol_8);
}
