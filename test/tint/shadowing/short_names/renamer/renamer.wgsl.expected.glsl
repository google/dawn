#version 310 es

vec4 v(uint v_1) {
  return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
void main() {
  gl_Position = v(uint(gl_VertexID));
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
