#version 310 es

uniform mat4 u[4];
float s;
float a(mat4 a_1[4]) {
  return a_1[0][0][0u];
}
float b(mat4 m) {
  return m[0][0u];
}
float c(vec4 v) {
  return v[0u];
}
float d(float f) {
  return f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_1 = a(u);
  float v_2 = (v_1 + b(u[1]));
  float v_3 = (v_2 + c(u[1][0].ywxz));
  s = (v_3 + d(u[1][0].ywxz[0u]));
}
