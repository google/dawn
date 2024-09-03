#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat4 u[4];
float16_t s;
float16_t a(f16mat4 a_1[4]) {
  return a_1[0][0][0u];
}
float16_t b(f16mat4 m) {
  return m[0][0u];
}
float16_t c(f16vec4 v) {
  return v[0u];
}
float16_t d(float16_t f) {
  return f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float16_t v_1 = a(u);
  float16_t v_2 = (v_1 + b(u[1]));
  float16_t v_3 = (v_2 + c(u[1][0].ywxz));
  s = (v_3 + d(u[1][0].ywxz[0u]));
}
