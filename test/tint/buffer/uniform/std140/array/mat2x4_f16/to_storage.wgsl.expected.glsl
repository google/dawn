#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x4_f16 {
  f16vec4 col0;
  f16vec4 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x4_f16 inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  f16mat2x4 inner[4];
} s;

f16mat2x4 conv_mat2x4_f16(mat2x4_f16 val) {
  return f16mat2x4(val.col0, val.col1);
}

f16mat2x4[4] conv_arr4_mat2x4_f16(mat2x4_f16 val[4]) {
  f16mat2x4 arr[4] = f16mat2x4[4](f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x4_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  s.inner = conv_arr4_mat2x4_f16(u.inner);
  s.inner[1] = conv_mat2x4_f16(u.inner[2u]);
  s.inner[1][0] = u.inner[0u].col1.ywxz;
  s.inner[1][0].x = u.inner[0u].col1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
