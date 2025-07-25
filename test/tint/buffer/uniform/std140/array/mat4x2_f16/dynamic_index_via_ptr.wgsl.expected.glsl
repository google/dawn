#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x2_f16_std140 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

layout(binding = 0, std140)
uniform a_block_std140_1_ubo {
  mat4x2_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
int counter = 0;
int i() {
  uint v_2 = uint(counter);
  counter = int((v_2 + uint(1)));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = min(uint(i()), 3u);
  f16mat4x2 v_4 = f16mat4x2(v.inner[v_3].col0, v.inner[v_3].col1, v.inner[v_3].col2, v.inner[v_3].col3);
  f16vec2 v_5 = v_4[min(uint(i()), 3u)];
  mat4x2_f16_std140 v_6[4] = v.inner;
  f16mat4x2 v_7[4] = f16mat4x2[4](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      v_7[v_9] = f16mat4x2(v_6[v_9].col0, v_6[v_9].col1, v_6[v_9].col2, v_6[v_9].col3);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  f16mat4x2 l_a[4] = v_7;
  f16mat4x2 l_a_i = v_4;
  f16vec2 l_a_i_i = v_5;
  v_1.inner = (((v_5.x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
