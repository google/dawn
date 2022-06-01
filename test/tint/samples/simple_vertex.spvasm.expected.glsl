#version 310 es

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  tint_symbol = vec4(0.0f);
  return;
}

struct main_out {
  vec4 tint_symbol;
};

main_out tint_symbol_1() {
  main_1();
  main_out tint_symbol_2 = main_out(tint_symbol);
  return tint_symbol_2;
}

void main() {
  gl_PointSize = 1.0;
  main_out inner_result = tint_symbol_1();
  gl_Position = inner_result.tint_symbol;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
