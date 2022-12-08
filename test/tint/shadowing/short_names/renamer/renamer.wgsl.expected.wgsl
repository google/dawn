@vertex
fn tint_symbol(@builtin(vertex_index) tint_symbol_1 : u32) -> @builtin(position) vec4f {
  return vec4f(vec2f(vec2i()), 0.0, 1.0);
}
