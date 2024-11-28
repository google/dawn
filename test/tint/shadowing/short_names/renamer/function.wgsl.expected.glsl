#version 310 es

int tint_symbol() {
  return 0;
}
float tint_symbol_1(int tint_symbol_2) {
  return float(tint_symbol_2);
}
bool tint_symbol_3(float tint_symbol_4) {
  return bool(tint_symbol_4);
}
vec4 tint_symbol_5_inner(uint tint_symbol_6) {
  return mix(vec4(0.0f), vec4(1.0f), bvec4(tint_symbol_3(tint_symbol_1(tint_symbol()))));
}
void main() {
  gl_Position = tint_symbol_5_inner(uint(gl_VertexID));
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
