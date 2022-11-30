#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x4_f16 {
  f16vec4 col0;
  f16vec4 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x4_f16 inner[4];
} u;

void a(f16mat2x4 a_1[4]) {
}

void b(f16mat2x4 m) {
}

void c(f16vec4 v) {
}

void d(float16_t f_1) {
}

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
  a(conv_arr4_mat2x4_f16(u.inner));
  b(conv_mat2x4_f16(u.inner[1u]));
  c(u.inner[1u].col0.ywxz);
  d(u.inner[1u].col0.ywxz[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
