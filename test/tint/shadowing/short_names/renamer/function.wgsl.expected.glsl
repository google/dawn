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
vec4 tint_symbol_inner(uint VertexIndex) {
  return mix(vec4(0.0f), vec4(1.0f), bvec4(vec2i(vec2f(vec4f()))));
}
void main() {
  gl_Position = tint_symbol_inner(uint(gl_VertexID));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
