#version 310 es

struct S {
  float a;
};

bool ret_bool() {
  return false;
}
int ret_i32() {
  return 0;
}
uint ret_u32() {
  return 0u;
}
float ret_f32() {
  return 0.0f;
}
ivec2 ret_v2i32() {
  return ivec2(0);
}
uvec3 ret_v3u32() {
  return uvec3(0u);
}
vec4 ret_v4f32() {
  return vec4(0.0f);
}
mat2x3 ret_m2x3() {
  return mat2x3(vec3(0.0f), vec3(0.0f));
}
float[4] ret_arr() {
  return float[4](0.0f, 0.0f, 0.0f, 0.0f);
}
S ret_struct() {
  return S(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
