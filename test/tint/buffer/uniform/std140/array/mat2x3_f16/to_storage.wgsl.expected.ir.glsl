#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat2x3 u[4];
f16mat2x3 s[4];
void tint_store_and_preserve_padding_1(inout f16mat2x3 target, f16mat2x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
}
void tint_store_and_preserve_padding(inout f16mat2x3 target[4], f16mat2x3 value_param[4]) {
  {
    uint v = 0u;
    v = 0u;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_1], value_param[v_1]);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(s, u);
  tint_store_and_preserve_padding_1(s[1], u[2]);
  s[1][0] = u[0][1].zxy;
  s[1][0][0u] = u[0][1].x;
}
