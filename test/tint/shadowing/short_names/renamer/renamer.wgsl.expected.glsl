#version 310 es

vec4 tint_symbol_inner(uint VertexIndex) {
  return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
void main() {
  gl_Position = tint_symbol_inner(uint(gl_VertexID));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
