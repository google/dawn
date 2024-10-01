#version 310 es


struct vec4f {
  int i;
};

vec4 tint_symbol_inner(uint VertexIndex) {
  vec4f s = vec4f(1);
  float f = float(s.i);
  bool b = bool(f);
  return mix(vec4(0.0f), vec4(1.0f), bvec4(b));
}
void main() {
  gl_Position = tint_symbol_inner(uint(gl_VertexID));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
