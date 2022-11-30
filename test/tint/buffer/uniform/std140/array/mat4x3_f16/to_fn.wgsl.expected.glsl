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

void a(f16mat4x3 a_1[4]) {
}

void b(f16mat4x3 m) {
}

void c(f16vec3 v) {
}

void d(float16_t f_1) {
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
  a(conv_arr4_mat4x3_f16(u.inner));
  b(conv_mat4x3_f16(u.inner[1u]));
  c(u.inner[1u].col0.zxy);
  d(u.inner[1u].col0.zxy[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
