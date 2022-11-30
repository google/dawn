#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct Inner {
  float16_t scalar_f16;
  uint pad;
  f16vec3 vec3_f16;
  f16mat2x4 mat2x4_f16;
};

struct Inner_std140 {
  float16_t scalar_f16;
  uint pad;
  f16vec3 vec3_f16;
  f16vec4 mat2x4_f16_0;
  f16vec4 mat2x4_f16_1;
};

struct S {
  Inner inner;
};

struct S_std140 {
  Inner_std140 inner;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  S_std140 inner;
} u;

Inner conv_Inner(Inner_std140 val) {
  return Inner(val.scalar_f16, val.pad, val.vec3_f16, f16mat2x4(val.mat2x4_f16_0, val.mat2x4_f16_1));
}

S conv_S(S_std140 val) {
  return S(conv_Inner(val.inner));
}

void tint_symbol() {
  S x = conv_S(u.inner);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
