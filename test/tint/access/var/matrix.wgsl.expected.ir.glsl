#version 310 es

float s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 v = m[1];
  float f = v[1];
  s = f;
}
