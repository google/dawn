#version 310 es


struct tint_symbol {
  int tint_symbol_1;
};

vec4 tint_symbol_4_inner(uint tint_symbol_5) {
  tint_symbol tint_symbol_6 = tint_symbol(1);
  float tint_symbol_7 = float(tint_symbol_6.tint_symbol_1);
  bool tint_symbol_8 = bool(tint_symbol_7);
  return mix(vec4(0.0f), vec4(1.0f), bvec4(tint_symbol_8));
}
void main() {
  gl_Position = tint_symbol_4_inner(uint(gl_VertexID));
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
