#version 310 es

struct VSOut {
  vec4 pos;
};

void foo(inout VSOut tint_symbol) {
  vec4 pos = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  tint_symbol.pos = pos;
}

VSOut tint_symbol_1() {
  VSOut tint_symbol = VSOut(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  foo(tint_symbol);
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VSOut inner_result = tint_symbol_1();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
