#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat2x3 u;
f16mat2x3 s;
void tint_store_and_preserve_padding(inout f16mat2x3 target, f16mat2x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(s, u);
  s[1] = u[0];
  s[1] = u[0].zxy;
  s[0][1] = u[1].x;
}
