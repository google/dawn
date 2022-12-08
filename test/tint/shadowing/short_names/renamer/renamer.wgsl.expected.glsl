#version 310 es

vec4 tint_symbol(uint VertexIndex) {
  return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
