#version 310 es


struct main_out {
  vec4 tint_symbol;
};

vec4 tint_symbol = vec4(0.0f);
void main_1() {
  tint_symbol = vec4(0.0f);
}
main_out tint_symbol_1_inner() {
  main_1();
  return main_out(tint_symbol);
}
void main() {
  gl_Position = tint_symbol_1_inner().tint_symbol;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
