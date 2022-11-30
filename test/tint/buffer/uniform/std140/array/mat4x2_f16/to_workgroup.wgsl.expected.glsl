#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x2_f16 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat4x2_f16 inner[4];
} u;

shared f16mat4x2 w[4];
f16mat4x2 conv_mat4x2_f16(mat4x2_f16 val) {
  return f16mat4x2(val.col0, val.col1, val.col2, val.col3);
}

f16mat4x2[4] conv_arr4_mat4x2_f16(mat4x2_f16 val[4]) {
  f16mat4x2 arr[4] = f16mat4x2[4](f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x2_f16(val[i]);
    }
  }
  return arr;
}

void f(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      w[i] = f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
    }
  }
  barrier();
  w = conv_arr4_mat4x2_f16(u.inner);
  w[1] = conv_mat4x2_f16(u.inner[2u]);
  w[1][0] = u.inner[0u].col1.yx;
  w[1][0].x = u.inner[0u].col1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
