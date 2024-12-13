#version 310 es

int v() {
  return 0;
}
float v_1(int v_2) {
  return float(v_2);
}
bool v_3(float v_4) {
  return bool(v_4);
}
vec4 v_5(uint v_6) {
  return mix(vec4(0.0f), vec4(1.0f), bvec4(v_3(v_1(v()))));
}
void main() {
  gl_Position = v_5(uint(gl_VertexID));
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
