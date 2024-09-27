#version 310 es

int vec4f() {
  return 0;
}

float vec2f(int i) {
  return float(i);
}

bool vec2i(float f) {
  return bool(f);
}

vec4 tint_symbol(uint VertexIndex) {
  vec4 tint_symbol_1 = vec4(0.0f);
  vec4 tint_symbol_2 = vec4(1.0f);
  int tint_symbol_3 = vec4f();
  float tint_symbol_4 = vec2f(tint_symbol_3);
  bool tint_symbol_5 = vec2i(tint_symbol_4);
  return mix(tint_symbol_1, tint_symbol_2, bvec4(tint_symbol_5));
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
