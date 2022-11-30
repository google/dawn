#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct S {
  int before;
  uint pad;
  f16mat4 m;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  int after;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
};

struct S_std140 {
  int before;
  uint pad;
  f16vec4 m_0;
  f16vec4 m_1;
  f16vec4 m_2;
  f16vec4 m_3;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  int after;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  S_std140 inner[4];
} u;

f16mat4 load_u_inner_2_m() {
  return f16mat4(u.inner[2u].m_0, u.inner[2u].m_1, u.inner[2u].m_2, u.inner[2u].m_3);
}

void f() {
  f16mat4 t = transpose(load_u_inner_2_m());
  float16_t l = length(u.inner[0u].m_1.ywxz);
  float16_t a = abs(u.inner[0u].m_1.ywxz[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
