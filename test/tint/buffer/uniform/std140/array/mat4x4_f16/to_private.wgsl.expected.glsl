#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x4_f16 {
  f16vec4 col0;
  f16vec4 col1;
  f16vec4 col2;
  f16vec4 col3;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat4x4_f16 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float16_t inner;
} s;

f16mat4 p[4] = f16mat4[4](f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
f16mat4 conv_mat4x4_f16(mat4x4_f16 val) {
  return f16mat4(val.col0, val.col1, val.col2, val.col3);
}

f16mat4[4] conv_arr4_mat4x4_f16(mat4x4_f16 val[4]) {
  f16mat4 arr[4] = f16mat4[4](f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x4_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  p = conv_arr4_mat4x4_f16(u.inner);
  p[1] = conv_mat4x4_f16(u.inner[2u]);
  p[1][0] = u.inner[0u].col1.ywxz;
  p[1][0].x = u.inner[0u].col1[0u];
  s.inner = p[1][0].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
