#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x3_f16 {
  f16vec3 col0;
  f16vec3 col1;
  f16vec3 col2;
  f16vec3 col3;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat4x3_f16 inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  f16mat4x3 inner[4];
} s;

void assign_and_preserve_padding_1_s_X(uint dest[1], f16mat4x3 value) {
  s.inner[dest[0]][0] = value[0u];
  s.inner[dest[0]][1] = value[1u];
  s.inner[dest[0]][2] = value[2u];
  s.inner[dest[0]][3] = value[3u];
}

void assign_and_preserve_padding_s(f16mat4x3 value[4]) {
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      uint tint_symbol[1] = uint[1](i);
      assign_and_preserve_padding_1_s_X(tint_symbol, value[i]);
    }
  }
}

f16mat4x3 conv_mat4x3_f16(mat4x3_f16 val) {
  return f16mat4x3(val.col0, val.col1, val.col2, val.col3);
}

f16mat4x3[4] conv_arr4_mat4x3_f16(mat4x3_f16 val[4]) {
  f16mat4x3 arr[4] = f16mat4x3[4](f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x3_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  assign_and_preserve_padding_s(conv_arr4_mat4x3_f16(u.inner));
  uint tint_symbol_1[1] = uint[1](1u);
  assign_and_preserve_padding_1_s_X(tint_symbol_1, conv_mat4x3_f16(u.inner[2u]));
  s.inner[1][0] = u.inner[0u].col1.zxy;
  s.inner[1][0].x = u.inner[0u].col1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
