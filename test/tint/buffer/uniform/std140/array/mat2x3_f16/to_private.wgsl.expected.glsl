#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x3_f16 {
  f16vec3 col0;
  f16vec3 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x3_f16 inner[4];
} u;

f16mat2x3 p[4] = f16mat2x3[4](f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
f16mat2x3 conv_mat2x3_f16(mat2x3_f16 val) {
  return f16mat2x3(val.col0, val.col1);
}

f16mat2x3[4] conv_arr4_mat2x3_f16(mat2x3_f16 val[4]) {
  f16mat2x3 arr[4] = f16mat2x3[4](f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x3_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  p = conv_arr4_mat2x3_f16(u.inner);
  p[1] = conv_mat2x3_f16(u.inner[2u]);
  p[1][0] = u.inner[0u].col1.zxy;
  p[1][0].x = u.inner[0u].col1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
