#version 310 es


struct VSOut {
  vec4 pos;
};

void foo(inout VSOut tint_symbol) {
  vec4 pos = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  tint_symbol.pos = pos;
}
VSOut tint_symbol_1_inner() {
  VSOut tint_symbol = VSOut(vec4(0.0f));
  foo(tint_symbol);
  return tint_symbol;
}
void main() {
  gl_Position = tint_symbol_1_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
